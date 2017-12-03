#include "face.h"
#include "ibex/ibex_IntervalVector.h"
/// ******************  Sepcialized ****************** ///
namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template <>
void Face<ibex::IntervalVector>::serialize(std::ofstream& binFile) const{
    // Face serialization
    serializeIntervalVector(binFile, m_position);
    serializeIntervalVector(binFile, m_orientation);
}

template <>
void Face<ibex::IntervalVector>::deserialize(std::ifstream& binFile){
    m_position = deserializeIntervalVector(binFile);
    m_orientation = deserializeIntervalVector(binFile);
}


template <>
const ibex::IntervalVector Face<ibex::IntervalVector>::get_position_typed() const{
    return m_position;
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
const ppl::C_Polyhedron Face<ppl::C_Polyhedron>::get_position_typed() const{
    return iv_2_polyhedron(this->m_position);
}

}
