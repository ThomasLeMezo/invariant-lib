#include "pave.h"
#include "room.h"

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template<>
int Pave<ibex::IntervalVector, ibex::IntervalVector>::get_dim_inter_boundary(const ibex::IntervalVector &box){
    IntervalVector intersection(get_dim(), ibex::Interval::EMPTY_SET);
    for(invariant::Face<ibex::IntervalVector, ibex::IntervalVector> *f:m_faces_vector){
        intersection |= (box & f->get_position());
    }
    return m_dim - invariant::get_nb_dim_flat<ibex::IntervalVector, ibex::IntervalVector>(intersection);
}

template<>
void Pave<ibex::IntervalVector, ibex::IntervalVector>::compute_typed_position(){
    m_position_typed = new ibex::IntervalVector(m_position);
}

/// ******************  ppl::C_Polyhedron ****************** ///

template<>
void Pave<ppl::C_Polyhedron, ppl::Generator_System>::compute_typed_position(){
    m_position_typed = new ppl::C_Polyhedron(iv_2_polyhedron(m_position));
    m_position_typed->minimized_constraints();
}

}
