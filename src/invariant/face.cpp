#include "face.h"
#include <serialization.h>

namespace invariant {

Face::Face(const ibex::IntervalVector &coordinates, const ibex::IntervalVector &orientation, Pave *p):
    m_coordinates(coordinates),
    m_orientation(orientation)
{
    m_pave = p;
}

Face::~Face(){

}

Face::Face(Pave *p):
    m_coordinates(0),
    m_orientation(0)
{
    m_pave = p;
}

ibex::IntervalVector Face::coordinates() const
{
    return m_coordinates;
}

std::ostream& operator<< (std::ostream& stream, const Face& f) {
    stream << f.coordinates();
    return stream;
}

void Face::serialize(std::ofstream& binFile) const{
    // Face serialization
    ibex_tools::serializeIntervalVector(binFile, m_coordinates);
    ibex_tools::serializeIntervalVector(binFile, m_orientation);
}

void Face::deserialize(std::ifstream& binFile){
    m_coordinates = ibex_tools::deserializeIntervalVector(binFile);
    m_orientation = ibex_tools::deserializeIntervalVector(binFile);
}

bool Face::operator==(const Face& f) const{
    if(m_coordinates == f.coordinates() && this->m_orientation==f.orientation())
        return true;
    else
        return false;
}

bool Face::operator!=(const Face& f) const{
    return !(*this == f);
}

ibex::IntervalVector Face::orientation() const
{
    return m_orientation;
}

}
