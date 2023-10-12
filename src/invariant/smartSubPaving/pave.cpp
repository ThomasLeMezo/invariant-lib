#include "pave.h"
#include "room.h"

using namespace std;
using namespace ibex;

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template<>
int PaveIBEX::get_dim_inter_boundary(const ibex::IntervalVector &box){
    IntervalVector intersection(get_dim(), ibex::Interval::EMPTY_SET);
    for(invariant::FaceIBEX *f:m_faces_vector){
        intersection |= (box & f->get_position());
    }
    return m_dim - invariant::get_nb_dim_flat(intersection);
}

template<>
void PaveIBEX::compute_typed_position(){
    m_position_typed = new ibex::IntervalVector(m_position);
}

/// ******************  invariant::ExpBox ****************** ///

template<>
int PaveEXP::get_dim_inter_boundary(const ibex::IntervalVector &box){
    IntervalVector intersection(get_dim(), ibex::Interval::EMPTY_SET);
    for(invariant::FaceEXP *f:m_faces_vector){
        intersection |= (box & f->get_position());
    }
    return m_dim - invariant::get_nb_dim_flat(intersection);
}

template<>
void PaveEXP::compute_typed_position(){
    m_position_typed = new ibex::IntervalVector(m_position);
}

/// ******************  ppl::C_Polyhedron ****************** ///

template<>
void PavePPL::compute_typed_position(){
    m_position_typed = new ppl::C_Polyhedron(iv_2_polyhedron(m_position));
    m_position_typed->minimized_constraints();
}

}
