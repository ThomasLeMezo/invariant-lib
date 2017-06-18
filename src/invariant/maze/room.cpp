#include "room.h"
#include "face.h"

using namespace ibex;
using namespace std;
namespace invariant {

Room::Room(Pave *p, Maze *m, Dynamics *dynamics)
{
    m_pave = p;
    m_maze = m;
    const IntervalVector position(p->get_position());
    vector<IntervalVector> vector_field_list = dynamics->eval(position);

    for(IntervalVector &vector_field:vector_field_list){
        // Test if 0 is inside the vector_field IV
        IntervalVector zero(m_maze->get_graph()->dim(), Interval::ZERO);
        if(!(zero.is_subset(vector_field)))
            m_vector_fields.push_back(vector_field);
    }

    int dim = m_pave->get_dim();
    for(int face=0; face<dim; face++){
        for(int sens=0; sens < 2; sens++){
            Door *d = new Door(m_pave->get_faces()[face][sens], this);
            Face *f = m_pave->get_faces()[face][sens];
            f->add_door(d);
        }
    }

    omp_init_lock(&m_lock_contraction);
    omp_init_lock(&m_lock_deque);
}

Room::~Room(){
    omp_destroy_lock(&m_lock_contraction);
    omp_destroy_lock(&m_lock_deque);
}

void Room::set_empty_output(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_output();
        f->get_doors()[m_maze]->synchronize();
    }
}

void Room::set_empty_private_input(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_input();
    }
}

void Room::set_empty(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private();
        f->get_doors()[m_maze]->synchronize();
    }
}

void Room::contract_vector_field(){
    int dim = m_pave->get_dim();
    for(IntervalVector &v:m_vector_fields){
        // Construct the boolean interval vector of the vector_field
        IntervalVector v_bool_in = IntervalVector::empty(dim);
        IntervalVector v_bool_out = IntervalVector::empty(dim);
        for(int i=0; i<dim; i++){
            if(!(v[i] & Interval::POS_REALS).is_empty())
                v_bool_out[0] |= Interval(1);
            if(!(v[0] & Interval::NEG_REALS).is_empty())
                v_bool_out[0] |= Interval(0);
            if(!(-v[i] & Interval::POS_REALS).is_empty())
                v_bool_in[0] |= Interval(1);
            if(!(-v[0] & Interval::NEG_REALS).is_empty())
                v_bool_in[0] |= Interval(0);
        }

        for(Face* f:m_pave->get_faces_vector()){
            Door *d = f->get_doors()[m_maze];
            if((f->get_orientation() & v_bool_in).is_empty())
                d->set_empty_private_input();
            if((f->get_orientation() & v_bool_out).is_empty())
                d->set_empty_private_output();
            // Note : synchronization will be proceed at the end of all contractors
            // to avoid unecessary lock
        }
    }
}

void Room::contract_consistency(){
    for(IntervalVector &vec_field:m_vector_fields){
        // Create tmp output doors
        const int dim = m_pave->get_dim();
        std::vector< std::array<IntervalVector, 2>> out_result;
        for(int i=0; i<dim; i++){
            std::array<IntervalVector, 2> output = {IntervalVector::empty(dim), IntervalVector::empty(dim)};
            out_result.push_back(output);
        }

        for(int face_in=0; face_in<dim; face_in++){
            for(int sens_in = 0; sens_in < 2; sens_in++){
                IntervalVector in_result(dim, Interval::EMPTY_SET);
                Face* f_in = m_pave->get_faces()[face_in][sens_in];
                Door* door_in = f_in->get_doors()[m_maze];
                const IntervalVector in(door_in->get_input_private());

                for(int face_out=0; face_out<dim; face_out++){
                    for(int sens_out = 0; sens_out < 2; sens_out++){
                        if(!(face_in==face_out && sens_in==sens_out)){
                            IntervalVector in_tmp(in);
                            Face* f_out = m_pave->get_faces()[face_out][sens_out];
                            Door* door_out = f_out->get_doors()[m_maze];
                            IntervalVector out_tmp(door_out->get_output_private());

                            this->contract_flow(in_tmp, out_tmp, vec_field);

                            in_result |= in_tmp;
                            out_result[face_out][sens_out] |= out_tmp;
                        }
                    }
                }
                door_in->set_input_private(in_result);
            }
        }

        for(int face_out =0; face_out<dim; face_out++){
            for(int sens_out = 0; sens_out < 2; sens_out++){
                Face* f_out = m_pave->get_faces()[face_out][sens_out];
                Door* door_out = f_out->get_doors()[m_maze];
                door_out->set_input_private(out_result[face_out][sens_out]);
            }
        }

    }
}

bool Room::contract_continuity(){
    if(m_vector_fields.size()==0)
        return false;
    bool change = false;
    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        change |= d->contract_continuity_private();
    }
    return change;
}

void Room::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect){
    // assert 0 not in v.
    IntervalVector c(out-in);
    IntervalVector v(vect);
    Interval alpha(Interval::POS_REALS);

    for(int i=0; i<v.size(); i++){
        alpha &= c[i]/v[i];
    }

    c &= alpha*v;
    out &= c+in;
    in &= out-c;
}

bool Room::contract(){
    omp_set_lock(&m_lock_contraction);
    bool change = false;
    if(m_first_contract){
        contract_vector_field();
        change = true;
    }
    change |= contract_continuity();

    if(change){
        contract_consistency();
    }
    m_first_contract = false;
    return change;
}

void Room::synchronize_doors(){
    for(Face* f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->synchronize();
    }
}

void Room::analyze_change(std::vector<Room *>&list_rooms){
    for(Face* f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        d->analyze_change(list_rooms);
    }
}

bool Room::is_empty(){
    if(m_empty)
        return true;
    else{
        bool empty = true;
        for(Face *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_empty()){
                empty = false;
                return false;
            }
        }
        m_empty = true;
        return true;
    }
}

bool Room::request_bisection(){
    return !(this->is_empty());
}
}
