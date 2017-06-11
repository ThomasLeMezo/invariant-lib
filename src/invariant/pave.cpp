#include "pave.h"

namespace invariant {

Pave::Pave(const ibex::IntervalVector &coordinates):
    m_coordinates(coordinates.size())
{
    m_coordinates = coordinates;

    // Build the faces

}

ibex::IntervalVector Pave::coordinates() const
{
    return m_coordinates;
}

std::vector<std::array<Face *, 2>> Pave::faces() const
{
    return m_faces;
}

Pave::~Pave(){
    for(std::array<Face *, 2> &a:m_faces){
        for(Face* f:a){
            delete(f);
        }
    }
}

std::ostream& operator<< (std::ostream& stream, const Pave& p) {
    stream << p.coordinates();
    return stream;
}

}
