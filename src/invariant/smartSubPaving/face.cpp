#include "face.h"
#include "ibex_IntervalVector.h"
/// ******************  Sepcialized ****************** ///
namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template <>
void Face<ibex::IntervalVector, ibex::IntervalVector>::serialize(std::ofstream& binFile) const{
    // Face serialization
    serializeIntervalVector(binFile, m_position);
    serializeIntervalVector(binFile, m_orientation);
}

template <>
void Face<ibex::IntervalVector, ibex::IntervalVector>::deserialize(std::ifstream& binFile){
    m_position = deserializeIntervalVector(binFile);
    m_orientation = deserializeIntervalVector(binFile);
}

template <>
Face<ibex::IntervalVector, ibex::IntervalVector>::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, Pave<ibex::IntervalVector, ibex::IntervalVector> *p):
    m_position(position),
    m_orientation(orientation),
    m_normal(normal),
    m_position_typed(position)
{
    m_pave = p;
}

template <>
Face<ibex::IntervalVector, ibex::IntervalVector>::Face(Pave<ibex::IntervalVector, ibex::IntervalVector> *p):
    m_position(0),
    m_orientation(0),
    m_normal(0),
    m_position_typed(0)
{
    m_pave = p;
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
Face<ppl::C_Polyhedron, ppl::Generator_System>::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, Pave<ppl::C_Polyhedron, ppl::Generator_System> *p):
    m_position(position),
    m_orientation(orientation),
    m_normal(normal)
{
    m_pave = p;
    m_position_typed = iv_2_polyhedron(position);
}

template <>
Face<ppl::C_Polyhedron, ppl::Generator_System>::Face(Pave<ppl::C_Polyhedron, ppl::Generator_System> *p):
    m_position(0),
    m_orientation(0),
    m_normal(0)
{
    m_pave = p;
}

}
