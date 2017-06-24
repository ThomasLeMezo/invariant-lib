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
        else
            m_vector_field_zero = true;
    }

    int dim = m_pave->get_dim();
    for(int face=0; face<dim; face++){
        for(int sens=0; sens < 2; sens++){
            Door *d = new Door(m_pave->get_faces()[face][sens], this);
            Face *f = m_pave->get_faces()[face][sens];
            f->add_door(d);
        }
    }

    m_full = false;
    m_empty = false;

    omp_init_lock(&m_lock_contraction);
    omp_init_lock(&m_lock_deque);
}

Room::~Room(){
    omp_destroy_lock(&m_lock_contraction);
    omp_destroy_lock(&m_lock_deque);
}

void Room::set_empty_private_output(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_output();
    }
}

void Room::set_empty_private_input(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private_input();
    }
}

void Room::set_full_private_output(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_output();
    }
}

void Room::set_full_private_input(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private_input();
    }
}

void Room::set_empty(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_private();
    }
}

void Room::set_full(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_full_private();
    }
}

void Room::contract_vector_field(){
    int dim = m_pave->get_dim();
    for(IntervalVector &v:m_vector_fields){
        // Construct the boolean interval vector of the vector_field
        IntervalVector v_bool_in = IntervalVector(dim, Interval::EMPTY_SET);
        IntervalVector v_bool_out = IntervalVector(dim, Interval::EMPTY_SET);

        for(int i=0; i<dim; i++){
            if(!(v[i] & Interval::POS_REALS).is_empty())
                v_bool_out[i] |= Interval(1);
            if(!(v[i] & Interval::NEG_REALS).is_empty())
                v_bool_out[i] |= Interval(0);
            if(!((-v[i]) & Interval::POS_REALS).is_empty())
                v_bool_in[i] |= Interval(1);
            if(!((-v[i]) & Interval::NEG_REALS).is_empty())
                v_bool_in[i] |= Interval(0);
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
    if(m_vector_field_zero && m_maze->get_type() == MAZE_PROPAGATOR){
        this->set_full();
        return;
    }

    MazeSens sens = m_maze->get_sens();
    MazeType type = m_maze->get_type();

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

                if(!in.is_empty()){
                    for(int face_out=0; face_out<dim; face_out++){
                        for(int sens_out = 0; sens_out < 2; sens_out++){
                            if(!(face_in==face_out && sens_in==sens_out)){
                                IntervalVector in_tmp(in);
                                Face* f_out = m_pave->get_faces()[face_out][sens_out];
                                Door* door_out = f_out->get_doors()[m_maze];
                                IntervalVector out_tmp(dim);
                                if(type == MAZE_CONTRACTOR)
                                    out_tmp = door_out->get_output_private();
                                else
                                    out_tmp = f_out->get_position();

                                if(!out_tmp.is_empty())
                                    this->contract_flow(in_tmp, out_tmp, vec_field);
                                else
                                    in_tmp.set_empty();

                                in_result |= in_tmp;
                                out_result[face_out][sens_out] |= out_tmp;
                            }
                        }
                    }
                    if(sens == MAZE_BWD || sens == MAZE_FWD_BWD){
                        if(type == MAZE_CONTRACTOR)
                            door_in->set_input_private(in & in_result);
                        else
                            door_in->set_input_private(in | in_result);
                    }
                }
            }
        }

        if(sens == MAZE_FWD || sens == MAZE_FWD_BWD){
            for(int face_out = 0; face_out<dim; face_out++){
                for(int sens_out = 0; sens_out < 2; sens_out++){
                    Face* f_out = m_pave->get_faces()[face_out][sens_out];
                    Door* door_out = f_out->get_doors()[m_maze];
                    if(type == MAZE_CONTRACTOR)
                        door_out->set_output_private(out_result[face_out][sens_out]);
                    else
                        door_out->set_output_private(door_out->get_output_private() | out_result[face_out][sens_out]);
                }
            }
        }

    }
}

bool Room::contract_continuity(){
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
        alpha &= ((c[i]/(v[i] & Interval::POS_REALS)) & Interval::POS_REALS) | ((c[i]/(v[i] & Interval::NEG_REALS)) & Interval::POS_REALS);
    }

    c &= alpha*v;

    MazeSens sens = m_maze->get_sens();
    if(sens == MAZE_FWD || sens == MAZE_FWD_BWD)
        out &= c+in;
    if(sens == MAZE_BWD || sens == MAZE_FWD_BWD)
        in &= out-c;
}

bool Room::contract(){
    bool change = false;
    if(m_maze->get_type() == MAZE_CONTRACTOR){
        if(m_vector_fields.size()==0)
            return false;
        if(m_first_contract){
            contract_vector_field();
            change = true;
        }
    }
    m_first_contract = false;

    change |= contract_continuity();

    if(change){
        contract_consistency();
    }

    return change;
}

void Room::synchronize_doors(){
    for(Face* f:m_pave->get_faces_vector()){
        Door *r = f->get_doors()[m_maze];
        r->synchronize();
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
        for(Face *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_empty()){
                return false;
            }
        }
        m_empty = true;
        return true;
    }
}

bool Room::is_full(){
    if(!m_full)
        return false;
    else{
        for(Face *f:m_pave->get_faces_vector()){
            if(!f->get_doors()[m_maze]->is_full()){
                m_full = false;
                return false;
            }
        }
        return true;
    }
}

bool Room::request_bisection(){
    if(m_maze->get_type() == MAZE_CONTRACTOR)
        return !(this->is_empty());
    else
        return !(this->is_empty());
}

std::ostream& operator<< (std::ostream& stream, const Room& r) {
    stream << "Room = " << r.get_pave()->get_position() << " - " << r.get_pave()->get_faces_vector().size() << " faces"<< endl;
    for(Face *f:r.get_pave()->get_faces_vector()){
        Door *d = f->get_doors()[r.get_maze()];
        stream << " Face : " << d->get_face()->get_orientation() << " - " << *d << endl;
    }
    return stream;
}

void Room::synchronize(){
    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        d->synchronize();
    }
}
}
