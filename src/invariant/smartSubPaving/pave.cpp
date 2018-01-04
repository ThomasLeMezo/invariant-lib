#include "pave.h"
#include "room.h"

using namespace std;
using namespace ibex;

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template<>
int Pave<ibex::IntervalVector>::get_dim_inter_boundary(const ibex::IntervalVector &box){
    IntervalVector intersection(get_dim(), ibex::Interval::EMPTY_SET);
    for(invariant::Face<ibex::IntervalVector> *f:m_faces_vector){
        intersection |= (box & f->get_position());
    }
    return m_dim - invariant::get_nb_dim_flat(intersection);
}

template<>
void Pave<ibex::IntervalVector>::compute_typed_position(){
    m_position_typed = new ibex::IntervalVector(m_position);
}

/// ******************  ppl::C_Polyhedron ****************** ///

template<>
void Pave<ppl::C_Polyhedron>::compute_typed_position(){
    m_position_typed = new ppl::C_Polyhedron(iv_2_polyhedron(m_position));
    m_position_typed->minimized_constraints();
}

}
