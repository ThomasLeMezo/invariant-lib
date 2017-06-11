#include "graph.h"
#include "serialization.h"

namespace invariant {
Graph::Graph(const ibex::IntervalVector &coordinates):
    m_coordinates(coordinates)
{

    // Create one Pave
    Pave* p = new Pave(coordinates);
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
    binFile.write((const char*)m_paves.size(), sizeof(size_t)); // Number of paves
    ibex_tools::serializeIntervalVector(binFile, m_coordinates);
    for(int i=0; i<(int)m_paves.size(); i++){
        Pave *p = m_paves[i];
        p->setSerialization_id(i);
        p->serialize(binFile);
    }
}

void Graph::deserialize(std::ifstream& binFile){
    m_paves.clear();

    size_t number_pave;
    binFile.read((char*)&number_pave, sizeof(size_t));
    m_coordinates = ibex_tools::deserializeIntervalVector(binFile);
    for(int i=0; i<(int)number_pave; i++){
        Pave *p = new Pave();
        m_paves.push_back(p);
    }
    for(int i=0; i<(int)number_pave; i++){
        Pave *p = m_paves[i];
        p->deserialize(binFile);
    }
}

std::ostream& operator<< (std::ostream& stream, const Graph& g) {
    stream << g.coordinates() << " " << g.paves().size() << " paves";
    return stream;
}

}
