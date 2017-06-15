#ifndef GRAPH_H
#define GRAPH_H

#include <ibex.h>
#include "pave.h"
#include "pave_node.h"
#include <fstream>

namespace invariant {
class Pave; // declared only for friendship
class Pave_node; // declared only for friendship
class Graph
{
public:
    /**
     * @brief Graph constructor with position
     * @param bounding box of the Graph
     */
    Graph(const ibex::IntervalVector &position);

    /**
     * @brief Empty Graph constructor
     */
    Graph():m_position(0){}

    /**
     * @brief Graph destructor
     */
    ~Graph();

    /**
     * @brief Equality between two graph
     * @param a Graph
     * @return true or false
     */
    const bool is_equal(const Graph& g) const;

    /**
     * @brief Return the i-th Pave of the bisectable pave list
     * @param i
     * @return
     */
    const Pave* operator[](std::size_t i) const;

    /**
     * @brief Return the coordinates of the Graph
     * @return an interval vector of the coordinates
     */
    const ibex::IntervalVector &get_position() const;

    /**
     * @brief Return the list of Paves of the Graph
     * @return A list of pointer to the paves
     */
    const std::vector<Pave *>& paves() const;

    /**
     * @brief serialize a Pave
     * @param binFile
     */
    void serialize(std::ofstream& binFile) const;

    /**
     * @brief deserialize a Pave
     * @param binFile
     */
    void deserialize(std::ifstream& binFile);

    /**
     * @brief Return the dimension of the space associated to the graph
     * @return dimension
     */
    const unsigned char& dim() const;

    /**
     * @brief bisect all the paves that ask to be bisected and update the graph
     */
    void bisect();

    /**
     * @brief Add Paves to the Graph list
     * @param p
     */
    void add_paves(Pave *p);

    /**
     * @brief Get the number of Paves in the bisectable and not_bisectable list
     * @return
     */
    const size_t size() const;

    /**
     * @brief Get the list of all paves that are not bisectable
     * @return
     */
    const std::vector<Pave *>& paves_not_bisectable() const;

    /**
     * @brief Getter to the root of the tree pave node
     * @return
     */
    Pave_node *get_pave_node();

private:
    /** Class Variable **/
    mutable ibex::IntervalVector      m_position; // Graph coordinates
    std::vector<Pave*>        m_paves; // Paves of the Graph
    std::vector<Pave*>        m_paves_not_bisectable; // Paves of the Graph
    mutable unsigned char             m_dim = 0; // Dimension of the space
    mutable Pave_node*                 m_pave_node; // Root of the pave node tree

};

    /**
     * @brief Overloading of the operator <<
     * @param stream
     * @param Graph
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const Graph& g);

}
#endif // GRAPH_H
