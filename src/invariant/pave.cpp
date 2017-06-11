#include "pave.h"
#include <serialization.h>

using namespace std;
using namespace ibex;

namespace invariant {

Pave::Pave(const ibex::IntervalVector &coordinates, Graph *g):
    m_coordinates(coordinates)
{
    m_graph = g;
    m_serialization_id = 0;
    unsigned char dim = g->dim();
    // Build the faces
    for(size_t i=0; i<dim; i++){
        IntervalVector iv_lb(m_coordinates);
        IntervalVector iv_ub(m_coordinates);
        IntervalVector orient_lb(dim, Interval(0, 1));
        IntervalVector orient_ub(dim, Interval(0,1));

        iv_lb[i]=Interval(m_coordinates[i].lb());
        iv_ub[i]=Interval(m_coordinates[i].ub());
        orient_lb[i] = Interval(0);
        orient_ub[i] = Interval(1);

        Face* face_lb = new Face(iv_lb, orient_lb, this);
        Face* face_ub = new Face(iv_ub, orient_ub, this);

        array<Face*, 2> face_array = {face_lb, face_ub};
        m_faces.push_back(face_array);
    }
}

Pave::Pave(Graph *g):
    m_coordinates(0)
{
    m_graph = g;
}

ibex::IntervalVector Pave::coordinates() const
{
    return m_coordinates;
}

std::vector<std::array<Face *, 2>> Pave::faces() const
{
    return m_faces;
}

size_t Pave::getSerialization_id() const
{
    return m_serialization_id;
}

void Pave::setSerialization_id(size_t &value)
{
    m_serialization_id = value;
}

Pave::~Pave(){
    for(std::array<Face *, 2> &a:m_faces){
        for(Face* f:a){
            delete(f);
        }
    }
}

void Pave::serialize(std::ofstream& binFile) const{
    // *** Pave serialization ***
    // size_t           Serialization id
    // IntervalVector   m_coordinates
    // [...] Faces

    binFile.write((const char*) &m_serialization_id, sizeof(size_t)); // Serialization id
    ibex_tools::serializeIntervalVector(binFile, m_coordinates);

    // Faces serialization
    for(size_t i=0; i<m_faces.size(); i++){
        for(unsigned char j=0; j<2; j++){
            m_faces[i][j]->serialize(binFile);
        }
    }
}

void Pave::deserialize(std::ifstream& binFile){
    binFile.read((char*)&m_serialization_id, sizeof(size_t));
    m_coordinates = ibex_tools::deserializeIntervalVector(binFile);

    // Create Faces
    for(unsigned char i=0; i<m_graph->dim(); i++){
        Face* face_lb = new Face(this);
        Face* face_ub = new Face(this);
        array<Face*, 2> face_array = {face_lb, face_ub};
        m_faces.push_back(face_array);
    }

    // Deserialize Faces & restore pointers link
    for(unsigned char i=0; i<m_graph->dim(); i++){
        m_faces[i][0]->deserialize(binFile);
        m_faces[i][1]->deserialize(binFile);
    }
}

std::ostream& operator<< (std::ostream& stream, const Pave& p) {
    stream << p.coordinates();
    return stream;
}

bool Pave::operator==(const Pave& p) const{
    if(m_coordinates != p.coordinates())
        return false;
    for(size_t i=0; i<m_faces.size(); i++){
        for(size_t j=0; j<m_faces[i].size(); j++){
            if(*(m_faces[i][j]) != *(p[i][j]))
                return false;
        }
    }
    return true;
}

bool Pave::operator!=(const Pave& p) const{
    return !(*this == p);
}

const std::array<Face*, 2>& Pave::operator[](std::size_t i) const{
    return m_faces[i];
}

}
