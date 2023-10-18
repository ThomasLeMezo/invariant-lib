#include "face.h"
#include "ibex_IntervalVector.h"

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template <>
void FaceIBEX::serialize(std::ofstream& binFile) const{
    // Face serialization
    serializeIntervalVector(binFile, m_position);
    serializeIntervalVector(binFile, m_orientation);
}

template <>
void FaceIBEX::deserialize(std::ifstream& binFile){
    m_position = deserializeIntervalVector(binFile);
    m_orientation = deserializeIntervalVector(binFile);
}

template <>
FaceIBEX::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, PaveIBEX *p):
    m_position(position),
    m_orientation(orientation),
    m_normal(normal),
    m_position_typed(position)
{
    m_pave = p;
    omp_init_lock(&m_write_neighbors);
}

template <>
FaceIBEX::Face(PaveIBEX *p):
    m_position(0),
    m_orientation(0),
    m_normal(0),
    m_position_typed(0)
{
    m_pave = p;
}

/// ******************  ExpBox : = IntervalVector ****************** ///

template <>
void FaceEXP::serialize(std::ofstream& binFile) const{
    // Face serialization
    serializeIntervalVector(binFile, m_position);
    serializeIntervalVector(binFile, m_orientation);
}

template <>
void FaceEXP::deserialize(std::ifstream& binFile){
    m_position = deserializeIntervalVector(binFile);
    m_orientation = deserializeIntervalVector(binFile);
}

template <>
FaceEXP::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, PaveEXP *p):
    m_position(position),
    m_orientation(orientation),
    m_normal(normal),
    m_position_typed(position)
{
    m_pave = p;
    omp_init_lock(&m_write_neighbors);
}

template <>
FaceEXP::Face(PaveEXP *p):
    m_position(0),
    m_orientation(0),
    m_normal(0),
    m_position_typed(m_position)
{
    m_pave = p;
}

/// ******************  ppl::C_Polyhedron ****************** ///

template <>
FacePPL::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, PavePPL *p):
    m_position(position),
    m_orientation(orientation),
    m_normal(normal)
{
    m_pave = p;
    m_position_typed = iv_2_polyhedron(position);
    omp_init_lock(&m_write_neighbors);
}

template <>
FacePPL::Face(PavePPL *p):
    m_position(0),
    m_orientation(0),
    m_normal(0)
{
    m_pave = p;
}

}
