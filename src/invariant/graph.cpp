#include "graph.h"

namespace invariant {
Graph::Graph(const ibex::IntervalVector &coordinates):
    m_coordinates(coordinates.size())
{
    m_coordinates = coordinates;

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

std::ostream& operator<< (std::ostream& stream, const Graph& g) {
    stream << g.coordinates() << " " << g.paves().size() << " paves";
    return stream;
}

}
