#include "face.h"
#include <serialization.h>
#include <iostream>

using namespace std;

namespace invariant {

Face::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, Pave *p):
    m_position(position),
    m_orientation(orientation)
{
    m_pave = p;
}

Face::~Face(){

}

Face::Face(Pave *p):
    m_position(0),
    m_orientation(0)
{
    m_pave = p;
}

const ibex::IntervalVector& Face::get_position() const
{
    return m_position;
}

std::ostream& operator<< (std::ostream& stream, const Face& f) {
    stream << f.get_position();
    return stream;
}

void Face::serialize(std::ofstream& binFile) const{
    // Face serialization
    ibex_tools::serializeIntervalVector(binFile, m_position);
    ibex_tools::serializeIntervalVector(binFile, m_orientation);
}

void Face::deserialize(std::ifstream& binFile){
    m_position = ibex_tools::deserializeIntervalVector(binFile);
    m_orientation = ibex_tools::deserializeIntervalVector(binFile);
}

const bool Face::is_equal(const Face& f) const{
    if(m_position == f.get_position() && this->m_orientation==f.get_orientation())
        return true;
    else
        return false;
}

const ibex::IntervalVector& Face::get_orientation() const{
    return m_orientation;
}

void Face::add_neighbor(Face *f){
    ibex::IntervalVector r = m_position & f->get_position();
    if(r.is_empty())
        return;
    int nb_not_flat = 0;
    for(int i=0; i<r.size(); i++){
        if(!r[i].is_degenerated())
                nb_not_flat++;
    }

    if(nb_not_flat == m_position.size()-1)
        m_neighbors.push_back(f);
}

void Face::remove_neighbor(const Face *f){
    for(size_t i=0; i<m_neighbors.size(); i++){
        if(m_neighbors[i] == f){ // pointer test
            m_neighbors.erase(m_neighbors.begin()+i);
            return;
        }
    }
    throw std::runtime_error("in [face.cpp/remove_neighobr] neighbor face was not found which is not expected");
}

const std::vector<Face *> &Face::get_neighbors() const{
    return m_neighbors;
}

}
