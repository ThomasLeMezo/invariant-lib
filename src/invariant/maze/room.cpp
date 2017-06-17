#include "room.h"

using namespace ibex;
using namespace std;
namespace invariant {

Room::Room(Pave *p, Maze *m, std::vector<ibex::Function*> f_vect)
{
    m_pave = p;
    m_maze = m;
    const IntervalVector position(p->get_position());
    for(Function*f:f_vect){
        IntervalVector vector_field = f->eval_vector(position);

        // Test if 0 is inside the vector_field IV
        IntervalVector zero(m_maze->get_graph()->dim(), Interval::ZERO);
        if(!(zero.is_subset(vector_field)))
            m_vector_fields.push_back(vector_field);
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
    m_first_contract = false;
}

//void Utils::CtcPaveBackward2(Pave *p, bool inclusion, std::vector<bool> &change_tab){
//    vector<IntervalVector> seg_out_list;
//    for(int i=0; i<4; i++)
//        seg_out_list.push_back(IntervalVector(2, Interval::EMPTY_SET));

//    for(int face = 0; face < 4; face++){
//        IntervalVector in(2, Interval::EMPTY_SET);
//        for(int j=(face+1)%4; j!=face; j=(j+1)%4){
//            IntervalVector seg_out(p->get_border(j)->get_segment_out_2D());
//            IntervalVector seg_in(p->get_border(face)->get_segment_in_2D());
//            this->CtcFlow(seg_in, seg_out, p->get_vector_field());
//            if(!seg_in[face%2].is_degenerated())
//                in |= seg_in;
//            if(!seg_out[j%2].is_degenerated())
//                seg_out_list[j] |= seg_out;
//        }

//        if(p->get_border(face)->get_segment_in_2D() != in)
//            change_tab[face] = true;
//        p->get_border(face)->set_segment_in(in[face%2], true);
//    }
//    for(int face=0; face <4; face++){
//        if(p->get_border(face)->get_segment_out_2D() != seg_out_list[face])
//            change_tab[face] = true;
//        p->get_border(face)->set_segment_out(seg_out_list[face][face%2], true);
//    }
//}

bool Room::contract_continuity(){
    bool change = false;
    for(Face *f:m_pave->get_faces_vector()){
        Door *d = f->get_doors()[m_maze];
        change |= d->contract_continuity_private();
    }
    return change;
}

void Room::contract_consistency(){

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
        synchronize_doors();
    }
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
}
