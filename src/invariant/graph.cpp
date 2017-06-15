#include "graph.h"
#include "serialization.h"
#include <stdexcept>

using namespace std;

namespace invariant {
Graph::Graph(const ibex::IntervalVector &position):
    m_position(position)
{
    m_dim = (unsigned char) position.size();

    // Create one Pave
    Pave* p = new Pave(position, this);
    m_pave_node = new Pave_node(p);
    p->set_pave_node(m_pave_node);
    m_paves.push_back(p);
}

Graph::~Graph(){
    for(Pave *p:m_paves){
        delete(p);
    }
    delete(m_pave_node);
}

void Graph::serialize(std::ofstream& binFile) const{
    // unsigned char    dimension
    // size_t           number of paves
    // IntervalVector   position
    // [...] Paves of the graph

    binFile.write((const char*)&m_dim, sizeof(unsigned char)); // dimension
    size_t size = m_paves.size();
    binFile.write((const char*)&size, sizeof(size_t)); // Number of paves
    ibex_tools::serializeIntervalVector(binFile, m_position);

    size_t cpt = 0;
    for(Pave *p:m_paves){
        p->set_serialization_id(cpt); cpt++;
        p->serialize(binFile);
    }
}

void Graph::deserialize(std::ifstream& binFile){
    if(m_paves.size()!=0){
        throw std::runtime_error("in [graph.cpp/deserialize] Graph is not empty");
        return;
    }

    binFile.read((char*)&m_dim, sizeof(unsigned char));
    size_t number_pave;
    binFile.read((char*)&number_pave, sizeof(size_t));
    m_position = ibex_tools::deserializeIntervalVector(binFile);

    const size_t number_pave_const = number_pave;
    for(size_t i=0; i<number_pave_const; i++){
        Pave *p = new Pave(this);
        m_paves.push_back(p);
    }
    for(size_t i=0; i<number_pave_const; i++){
        Pave *p = m_paves[i];
        p->deserialize(binFile);
    }
}

const bool Graph::is_equal(const Graph& g) const{
    if(m_position != g.get_position())
        return false;
    if(m_dim != g.dim())
        return false;
    const size_t nb_pave = m_paves.size();
    for(size_t i=0; i<nb_pave; i++){
        if(!(m_paves[i]->is_equal(*(g[i]))))
            return false;
    }
    return true;
}

void Graph::bisect(){
    vector<Pave*> m_bisectable_paves = m_paves;
    vector<Pave*> m_bisected_paves;
    m_paves.clear();

    /// Bisect the graph ///
    while(m_bisectable_paves.size()>0){
        Pave *p = m_bisectable_paves.back();
        m_bisectable_paves.pop_back();

        if(p->request_bisection()){
            p->bisect(); // bisected added to m_paves
            m_bisected_paves.push_back(p);
        }
        else{
            // Store not bisectable paves
            m_paves_not_bisectable.push_back(p);
        }
    }

    /// Call maze update (parallel loop possible) ///

    // ToDo

    /// Delete parent of bisected paves ///
    for(Pave* p:m_bisected_paves)
        delete(p);

}

}
