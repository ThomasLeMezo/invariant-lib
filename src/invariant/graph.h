#ifndef GRAPH_H
#define GRAPH_H

#include <ibex.h>
#include "pave.h"

namespace invariant {
class Graph
{
public:
    /**
     * @brief Graph constructor with coordinates
     * @param bounding box of the Graph
     */
    Graph(const ibex::IntervalVector &coordinates);

    ~Graph();

    /**
     * @brief Return the coordinates of the Graph
     * @return an interval vector of the coordinates
     */
    ibex::IntervalVector coordinates() const;

    /**
     * @brief Return the list of Paves of the Graph
     * @return A list of pointer to the paves
     */
    std::vector<Pave *> paves() const;

    void serialize(std::ofstream &binFile) const;
    void deserialize(std::ifstream& binFile);

private:
    /** Class Variable **/
    ibex::IntervalVector      m_coordinates; // Graph coordinates
    std::vector<Pave*>        m_paves; // Paves of the Graph
};

    /**
     * @brief Overloading of operator <<
     * @param stream
     * @param Graph
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const Graph& g);

}
#endif // GRAPH_H
