#include "face.h"

namespace invariant {

Face::Face(const ibex::IntervalVector &coordinates, const ibex::IntervalVector &orientation):
    m_coordinates(coordinates.size()),
    m_orientation(coordinates.size())
{
    m_coordinates = coordinates;
    m_orientation = orientation;
}

ibex::IntervalVector Face::coordinates() const
{
    return m_coordinates;
}

std::ostream& operator<< (std::ostream& stream, const Face& f) {
    stream << f.coordinates();
    return stream;
}

}
