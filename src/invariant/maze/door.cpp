#include "door.h"

namespace ppl=Parma_Polyhedra_Library;
using namespace ibex;
using namespace std;

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template <>
void DoorIBEX::set_empty_private_output(){
    m_output_private->set_empty();
}

template <>
void DoorIBEX::set_empty_private_input(){
    m_input_private->set_empty();
}

template <> DoorIBEX::Door(invariant::FaceIBEX *face, invariant::RoomIBEX *room):
    m_input_public(face->get_position()),
    m_output_public(face->get_position())
{
    m_input_private = new IntervalVector(face->get_position());
    m_output_private = new IntervalVector(face->get_position());

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}

template <>
void DoorIBEX::set_input_private(const ibex::IntervalVector& iv_input){
    *m_input_private = iv_input;
}

template <>
void DoorIBEX::set_output_private(const ibex::IntervalVector& iv_output){
    *m_output_private = iv_output;
}

template <>
void DoorIBEX::synchronize(){
    omp_set_lock(&m_lock_read);
    m_input_public = *m_input_private;
    m_output_public = *m_output_private;
    omp_unset_lock(&m_lock_read);
}

/// ******************  invariant::ExpBox ****************** ///

template <>
void DoorEXP::set_empty_private_output(){
    m_output_private->set_empty();
}

template <>
void DoorEXP::set_empty_private_input(){
    m_input_private->set_empty();
}

template <> DoorEXP::Door(invariant::FaceEXP *face, invariant::RoomEXP *room):
    m_input_public(face->get_position()),
    m_output_public(face->get_position())
{
    m_input_private = new invariant::ExpPoly(face->get_position());
    m_output_private = new invariant::ExpPoly(face->get_position());

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}

template <>
void DoorEXP::set_input_private
			(const invariant::ExpPoly& iv_input){
    *m_input_private = iv_input;
}

template <>
void DoorEXP::set_output_private
			(const invariant::ExpPoly& iv_output){
    *m_output_private = iv_output;
}

template <>
void DoorEXP::synchronize(){
    omp_set_lock(&m_lock_read);
    m_input_public = *m_input_private;
    m_output_public = *m_output_private;
    omp_unset_lock(&m_lock_read);
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
void DoorPPL::set_empty_private_output(){
    *m_output_private = ppl::C_Polyhedron(m_face->get_pave()->get_dim(), ppl::EMPTY);
}

template <>
void DoorPPL::set_empty_private_input(){
    *m_input_private = ppl::C_Polyhedron(m_face->get_pave()->get_dim(), ppl::EMPTY);
}

template <> DoorPPL::Door(invariant::FacePPL *face, invariant::RoomPPL *room)
{
    m_input_public = iv_2_polyhedron(face->get_position());
    m_input_public.minimized_constraints();
    m_output_public = m_input_public;
    m_input_private = new C_Polyhedron(m_input_public);
    m_output_private = new C_Polyhedron(m_input_public);

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}

template <>
void DoorPPL::set_input_private(const Parma_Polyhedra_Library::C_Polyhedron& iv_input){
    *m_input_private = iv_input;
}

template <>
void DoorPPL::set_output_private(const Parma_Polyhedra_Library::C_Polyhedron& iv_output){
    *m_output_private = iv_output;
}

template <>
void DoorPPL::synchronize(){
    m_input_private->minimized_constraints();
    m_output_private->minimized_constraints();
    omp_set_lock(&m_lock_read);
    m_input_public = *m_input_private;
    m_output_public = *m_output_private;
    omp_unset_lock(&m_lock_read);
}

/// ******************  Other functions ****************** ///

ppl::C_Polyhedron iv_2_polyhedron(const ibex::IntervalVector& iv){
    size_t iv_size = (size_t) iv.size();
    ppl::Rational_Box box(iv_size);
    if(!iv.is_empty()){
        for(size_t i=0; i<iv_size; i++){
            ppl::Variable x(i);
            if(!std::isinf(iv[i].lb())){ // '::' To avoid bug with Travis
                mpq_class lb(iv[i].lb());
                box.add_constraint(x*lb.get_den() >= lb.get_num());
            }
            if(!std::isinf(iv[i].ub())){
                mpq_class ub(iv[i].ub());
                box.add_constraint(x*ub.get_den() <= ub.get_num());
            }
        }
    }
    else{
        box = ppl::Rational_Box(iv.size(), Parma_Polyhedra_Library::Degenerate_Element::EMPTY);
    }
    return ppl::C_Polyhedron(box);
}

ibex::IntervalVector polyhedron_2_iv(const ppl::C_Polyhedron& p){
    ppl::Rational_Box box(p);
    ibex::IntervalVector result(p.space_dimension(), ibex::Interval::EMPTY_SET);

    for(size_t i=0; i<box.space_dimension(); i++){
        ppl::Variable x(i);
        result[i] = ibex::Interval(box.get_interval(x).lower().get_d(), box.get_interval(x).upper().get_d());
    }
    return result;
}



}

