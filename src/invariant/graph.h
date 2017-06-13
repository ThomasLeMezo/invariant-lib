#ifndef GRAPH_H
#define GRAPH_H

#include <ibex.h>
#include "pave.h"
#include <fstream>

namespace invariant {
class Pave; // declared only for friendship
class Graph
{
public:
    /**
     * @brief Graph constructor with coordinates
     * @param bounding box of the Graph
     */
    Graph(const ibex::IntervalVector &coordinates);
    Graph():m_coordinates(0){}

    ~Graph();

    /**
     * @brief Equality between two graph (overloading operator ==)
     * @param a Graph
     * @return true or false
     */
    bool operator==(const Graph& g) const;
    bool operator!=(const Graph& g) const;
    Pave* operator[](std::size_t i) const;

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

    void serialize(std::ofstream& binFile) const;
    void deserialize(std::ifstream& binFile);

    unsigned char dim() const;

    void bisect();
    void add_paves(Pave *p);
    size_t size();

private:
    /** Class Variable **/
    ibex::IntervalVector      m_coordinates; // Graph coordinates
    std::vector<Pave*>        m_paves; // Paves of the Graph
    std::vector<Pave*>        m_paves_not_bisectable; // Paves of the Graph
    unsigned char             m_dim = 0; // Dimension of the space
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
