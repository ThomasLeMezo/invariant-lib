#include "door.h"
#include "ppl.hh"

namespace ppl=Parma_Polyhedra_Library;

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template <>
void Door<ibex::IntervalVector>::set_empty_private_output(){
    m_output_private->set_empty();
}

template <>
void Door<ibex::IntervalVector>::set_empty_private_input(){
    m_input_private->set_empty();
}

template <> Door<ibex::IntervalVector>::Door(invariant::Face<ibex::IntervalVector> *face, invariant::Room<ibex::IntervalVector> *room):
    m_input_public(face->get_position()),
    m_output_public(face->get_position())
{
    m_input_private = new IntervalVector(face->get_position());
    m_output_private = new IntervalVector(face->get_position());

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}

template <> bool Door<ibex::IntervalVector>::contract_continuity_private(){
    Dynamics::DYNAMICS_SENS dynamics_sens = m_room->get_maze()->get_dynamics()->get_sens();
    typename Domain<ibex::IntervalVector>::DOMAIN_INITIALIZATION domain_init = m_room->get_maze()->get_domain()->get_init();

    bool change = false;

    if(dynamics_sens == Dynamics::FWD || dynamics_sens == Dynamics::FWD_BWD){
        IntervalVector door_input = ibex::IntervalVector(m_input_private->size(), ibex::Interval::EMPTY_SET);
        for(Face<ibex::IntervalVector>* f:m_face->get_neighbors()){
            Door<ibex::IntervalVector> *d = f->get_doors()[m_room->get_maze()];
            door_input |= (d->get_output() & m_face->get_position());
        }
        if(door_input != *m_input_private){
            change = true;
            if(domain_init == Domain<ibex::IntervalVector>::FULL_DOOR)
                (*m_input_private) &= door_input;
            else if(domain_init == Domain<ibex::IntervalVector>::FULL_WALL)
                (*m_input_private) |= door_input;
        }
    }

    if(dynamics_sens == Dynamics::BWD || dynamics_sens == Dynamics::FWD_BWD){
        IntervalVector door_output = ibex::IntervalVector(m_output_private->size(), ibex::Interval::EMPTY_SET);
        for(Face<ibex::IntervalVector>* f:m_face->get_neighbors()){
            Door<ibex::IntervalVector> *d = f->get_doors()[m_room->get_maze()];
            door_output |= (d->get_input() & m_face->get_position());
        }
        if(door_output != (*m_output_private)){
            change = true;
            if(domain_init == Domain<ibex::IntervalVector>::FULL_DOOR)
                (*m_output_private) &= door_output;
            else if(domain_init == Domain<ibex::IntervalVector>::FULL_WALL)
                (*m_output_private) |= door_output;
        }
    }
    return change;
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
void Door<ppl::C_Polyhedron>::set_empty_private_output(){
    *m_output_private = ppl::C_Polyhedron(m_face->get_pave()->get_dim()-1, ppl::EMPTY);
}

template <>
void Door<ppl::C_Polyhedron>::set_empty_private_input(){
    *m_input_private = ppl::C_Polyhedron(m_face->get_pave()->get_dim()-1, ppl::EMPTY);
}

template <> Door<ppl::C_Polyhedron>::Door(invariant::Face<ppl::C_Polyhedron> *face, invariant::Room<ppl::C_Polyhedron> *room)
{
    m_input_public = iv_2_polyhedron(face->get_position());
    m_output_public = m_input_public;
    m_input_private = new C_Polyhedron(m_input_public);
    m_output_private = new C_Polyhedron(m_input_public);

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}



/// ******************  Other functions ****************** ///

// Define operator | and & for C_Polyhedron

ppl::C_Polyhedron& operator&=(ppl::C_Polyhedron& p1, const ppl::C_Polyhedron& p2){
    p1.intersection_assign(p2);
    return p1;
}

ppl::C_Polyhedron& operator|=(ppl::C_Polyhedron& p1, const ppl::C_Polyhedron& p2){
    p1.poly_difference_assign(p2);
    return p1;
}

ppl::C_Polyhedron iv_2_polyhedron(const ibex::IntervalVector& iv){
        Rational_Box box(iv.size());
        for(size_t i=0; i<box.space_dimension(); i++){
            ppl::Variable x(i);
            if(!iv[i].is_empty()){
                box.add_constraint(x >= floor(iv[i].lb()*IBEX_PPL_PRECISION));
                box.add_constraint(x <= ceil(iv[i].ub()*IBEX_PPL_PRECISION));
            }
        }
        return ppl::C_Polyhedron(box);
}

}
