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

void Face::serialize(std::ofstream& binFile) const{
    // Face serialization
    ibex_tools::serializeIntervalVector(binFile, m_position);
    ibex_tools::serializeIntervalVector(binFile, m_orientation);
}

void Face::deserialize(std::ifstream& binFile){
    m_position = ibex_tools::deserializeIntervalVector(binFile);
    m_orientation = ibex_tools::deserializeIntervalVector(binFile);
}

void Face::add_neighbor(Face *f){
    const ibex::IntervalVector r = m_position & f->get_position();
    int nb_not_flat = 0;
    for(int i=0; i<r.size(); i++){
        if(r[i].is_degenerated()){
            nb_not_flat++;
            if(nb_not_flat>1)
                return;
        }
    }
    m_neighbors.push_back(f);
}

void Face::remove_neighbor(const Face *f){
    const size_t nb_neighbor = m_neighbors.size();
    for(size_t i=0; i<nb_neighbor; i++){
        if(m_neighbors[i] == f){ // pointer test
            m_neighbors.erase(m_neighbors.begin()+i);
            return;
        }
    }
    throw std::runtime_error("in [face.cpp/remove_neighobr] neighbor face was not found which is not expected");
}

std::ostream& operator<< (std::ostream& stream, const std::vector<Face*> &l){
    for(Face *f:l){
        stream << *f << endl;
    }
    return stream;
}

}
