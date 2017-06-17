#ifndef GRAPH_H
#define GRAPH_H

#include <ibex.h>
#include "pave.h"
#include "pave_node.h"
#include <fstream>
#include "maze.h"

namespace invariant {
class Pave; // declared only for friendship
class Pave_node; // declared only for friendship
class Maze; // declared only for friendship
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
    const std::vector<Pave *>& get_paves() const;

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
    const std::vector<Pave *>& get_paves_not_bisectable() const;

    /**
     * @brief Getter to the root of the tree pave node
     * @return
     */
    Pave_node *get_pave_node() const;

    /**
     * @brief Getter to the vector of mazes
     * @return
     */
    const std::vector<Maze *>& get_mazes() const;

    /**
     * @brief Add a new maze to the vector of mazes
     * @param maze
     * @param room
     */
    void add_maze(Maze * maze);

private:
    /** Class Variable **/
    mutable ibex::IntervalVector    m_position; // Graph coordinates
    std::vector<Pave*>              m_paves; // Paves of the Graph
    std::vector<Pave*>              m_paves_not_bisectable; // Paves of the Graph
    mutable unsigned char           m_dim = 0; // Dimension of the space
    mutable Pave_node*              m_pave_node = NULL; // Root of the pave node tree

    std::vector<Maze*>              m_mazes;

};

    /**
     * @brief Overloading of the operator <<
     * @param stream
     * @param Graph
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const Graph& g);

}

/// ***** Inline functions *****///

namespace invariant{
inline Pave_node* Graph::get_pave_node() const{
    return m_pave_node;
}

inline const std::vector<Pave *> &Graph::get_paves_not_bisectable() const{
    return m_paves_not_bisectable;
}

inline const size_t Graph::size() const{
    return m_paves.size() + m_paves_not_bisectable.size();
}

inline const Pave* Graph::operator[](std::size_t i) const{
    return m_paves[i];
}

inline const unsigned char& Graph::dim() const{
    return m_dim;
}

inline std::ostream& operator<< (std::ostream& stream, const Graph& g) {
    stream << g.get_position() << " " << g.get_paves().size() << " paves";
    return stream;
}
inline const ibex::IntervalVector& Graph::get_position() const{
    return m_position;
}

inline const std::vector<Pave *> &Graph::get_paves() const{
    return m_paves;
}

inline void Graph::add_paves(Pave *p){
    m_paves.push_back(p);
}

inline const std::vector<Maze *>& Graph::get_mazes() const{
    return m_mazes;
}

inline void Graph::add_maze(Maze * maze){
    m_mazes.push_back(maze);
}

}
#endif // GRAPH_H
