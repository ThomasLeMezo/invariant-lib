#include "pave.h"

namespace invariant {

Pave::Pave(const ibex::IntervalVector &coordinates):
    m_coordinates(coordinates)
{

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

int Pave::getSerialization_id() const
{
    return serialization_id;
}

void Pave::setSerialization_id(int value)
{
    serialization_id = value;
}

Pave::~Pave(){
    for(std::array<Face *, 2> &a:m_faces){
        for(Face* f:a){
            delete(f);
        }
    }
}

void Pave::serialize(std::ofstream &binFile) const{

}

void Pave::deserialize(std::ifstream& binFile){

}

std::ostream& operator<< (std::ostream& stream, const Pave& p) {
    stream << p.coordinates();
    return stream;
}

}
