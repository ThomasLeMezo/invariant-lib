#include "graph.h"
#include "serialization.h"

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
    m_paves.clear();

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

bool Graph::operator==(const Graph& g) const{
    if(m_coordinates != g.coordinates())
        return false;
    if(m_dim != g.dim())
        return false;
    for(size_t i=0; i<m_paves.size(); i++){
        if(*(m_paves[i]) != *(g[i]))
            return false;
    }
    return true;
}

bool Graph::operator!=(const Graph& g) const{
    return !(*this == g);
}

Pave* Graph::operator[](std::size_t i) const{
    return m_paves[i];
}
}
