#ifndef GRAPH_H
#define GRAPH_H

#include <ibex/ibex_IntervalVector.h>

#include "pave.h"
#include "pave_node.h"
#include <fstream>
#include "maze.h"

namespace invariant {
class Pave; // declared only for friendship
class Pave_node; // declared only for friendship
class Maze; // declared only for friendship
class SmartSubPaving
{
public:
    /**
     * @brief SmartSubPaving constructor with position
     * @param bounding box of the SmartSubPaving
     */
    SmartSubPaving(const ibex::IntervalVector &space);

    /**
     * @brief Empty SmartSubPaving constructor
     */
    SmartSubPaving():m_position(0){}

    /**
     * @brief SmartSubPaving destructor
     */
    ~SmartSubPaving();

    /**
     * @brief Equality between two paving
     * @param a SmartSubPaving
     * @return true or false
     */
    const bool is_equal(const SmartSubPaving& g) const;

    /**
     * @brief Return the i-th Pave of the bisectable pave list
     * @param i
     * @return
     */
    const Pave* operator[](std::size_t i) const;

    /**
     * @brief Return the coordinates of the SmartSubPaving
     * @return an interval vector of the coordinates
     */
    const ibex::IntervalVector &get_position() const;

    /**
     * @brief Return the list of Paves of the SmartSubPaving
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
     * @brief Return the dimension of the space associated to the paving
     * @return dimension
     */
    const unsigned char& dim() const;

    /**
     * @brief bisect all the paves that ask to be bisected and update the paving
     */
    void bisect();

    /**
     * @brief Add Paves to the SmartSubPaving list
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
    Pave_node *get_tree() const;

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

    /**
     * @brief Get info of all room inside position
     * @param maze
     * @param position
     * @return
     */
    void get_room_info(Maze* maze, const ibex::IntervalVector &position, std::vector<Pave *> &pave_list) const;

    /**
     * @brief Delete a pave for memory optimization
     * @param id
     */
    void delete_pave(int id);

    /**
     * @brief Bisector strategy (bisect according to ratio dimension of the search space)
     * @param position
     * @return
     */
    std::pair<ibex::IntervalVector, ibex::IntervalVector> bisect_largest_first(const ibex::IntervalVector &position);

    /**
     * @brief Set the limit of bisection (avoid to bisect if possible)
     * @param limit_bisection
     */
    void set_limit_bisection(const std::vector<double> &limit_bisection);

protected:
    /** Class Variable **/
    mutable ibex::IntervalVector    m_position; // SmartSubPaving coordinates
    std::vector<Pave*>              m_paves; // Paves of the SmartSubPaving
    std::vector<Pave*>              m_paves_not_bisectable; // Paves of the SmartSubPaving
    mutable unsigned char           m_dim = 0; // Dimension of the space
    mutable Pave_node*              m_tree = NULL; // Root of the pave node tree

    std::vector<Maze*>              m_mazes;

    std::vector<double> m_ratio_dimension;
    std::vector<double> m_limit_bisection;

};

    /**
     * @brief Overloading of the operator <<
     * @param stream
     * @param SmartSubPaving
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const SmartSubPaving& g);

}

/// ***** Inline functions *****///

namespace invariant{
inline Pave_node* SmartSubPaving::get_tree() const{
    return m_tree;
}

inline const std::vector<Pave *> &SmartSubPaving::get_paves_not_bisectable() const{
    return m_paves_not_bisectable;
}

inline const size_t SmartSubPaving::size() const{
    return m_paves.size() + m_paves_not_bisectable.size();
}

inline const Pave* SmartSubPaving::operator[](std::size_t i) const{
    return m_paves[i];
}

inline const unsigned char& SmartSubPaving::dim() const{
    return m_dim;
}

inline std::ostream& operator<< (std::ostream& stream, const SmartSubPaving& g) {
    stream << "GRAPH : " << g.get_position() << " - " << g.get_paves().size() << " paves";
    return stream;
}
inline const ibex::IntervalVector& SmartSubPaving::get_position() const{
    return m_position;
}

inline const std::vector<Pave *> &SmartSubPaving::get_paves() const{
    return m_paves;
}

inline void SmartSubPaving::add_paves(Pave *p){
    m_paves.push_back(p);
}

inline const std::vector<Maze *>& SmartSubPaving::get_mazes() const{
    return m_mazes;
}

inline void SmartSubPaving::add_maze(Maze * maze){
    m_mazes.push_back(maze);
}

inline void SmartSubPaving::set_limit_bisection(const std::vector<double> &limit_bisection){
    if(limit_bisection.size() != m_dim)
        throw std::runtime_error("in [paving.cpp/set_limit_bisection] dimensions doesn't match");
    else
        m_limit_bisection = limit_bisection;
}

}
#endif // GRAPH_H
