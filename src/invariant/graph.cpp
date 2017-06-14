#include "graph.h"
#include "serialization.h"
#include <stdexcept>

using namespace std;

namespace invariant {
Graph::Graph(const ibex::IntervalVector &coordinates):
    m_coordinates(coordinates)
{
    m_dim = (unsigned char) coordinates.size();

    // Create one Pave
    Pave* p = new Pave(coordinates, this);
    m_paves.push_back(p);
}

Graph::~Graph(){
    for(Pave *p:m_paves){
        delete(p);
    }
}

ibex::IntervalVector Graph::coordinates() const
{
    return m_coordinates;
}

std::vector<Pave *> Graph::paves() const
{
    return m_paves;
}

void Graph::add_paves(Pave *p){
    m_paves.push_back(p);
}

void Graph::serialize(std::ofstream& binFile) const{
    // unsigned char    dimension
    // size_t           number of paves
    // IntervalVector   coordinates
    // [...] Paves of the graph

    binFile.write((const char*)&m_dim, sizeof(unsigned char)); // dimension
    size_t size = m_paves.size();
    binFile.write((const char*)&size, sizeof(size_t)); // Number of paves
    ibex_tools::serializeIntervalVector(binFile, m_coordinates);

    size_t cpt = 0;
    for(Pave *p:m_paves){
        p->setSerialization_id(cpt); cpt++;
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
    m_coordinates = ibex_tools::deserializeIntervalVector(binFile);
    for(size_t i=0; i<number_pave; i++){
        Pave *p = new Pave(this);
        m_paves.push_back(p);
    }
    for(size_t i=0; i<number_pave; i++){
        Pave *p = m_paves[i];
        p->deserialize(binFile);
    }
}

unsigned char Graph::dim() const
{
    return m_dim;
}

std::ostream& operator<< (std::ostream& stream, const Graph& g) {
    stream << g.coordinates() << " " << g.paves().size() << " paves";
    return stream;
}

bool Graph::is_equal(const Graph& g) const{
    if(m_coordinates != g.coordinates())
        return false;
    if(m_dim != g.dim())
        return false;
    for(size_t i=0; i<m_paves.size(); i++){
        if(m_paves[i]->is_not_equal(*(g[i])))
            return false;
    }
    return true;
}

bool Graph::is_not_equal(const Graph& g) const{
    return !is_equal(g);
}

Pave* Graph::operator[](std::size_t i) const{
    return m_paves[i];
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

size_t Graph::size(){
    return m_paves.size() + m_paves_not_bisectable.size();
}

std::vector<Pave *> Graph::paves_not_bisectable() const
{
    return m_paves_not_bisectable;
}
}
