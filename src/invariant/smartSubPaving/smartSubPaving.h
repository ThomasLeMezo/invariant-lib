#ifndef GRAPH_H
#define GRAPH_H

#include <fstream>
#include <stdexcept>

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "pave.h"
#include "pave_node.h"
#include "maze.h"

#include "../serialization/ibex_serialization.h"

namespace invariant {

using SmartSubPavingPPL = SmartSubPaving<Parma_Polyhedra_Library::C_Polyhedron>;
using SmartSubPavingIBEX = SmartSubPaving<ibex::IntervalVector>;

enum BISECTION_STRATEGY{BISECTION_STANDARD, BISECTION_LB, BISECTION_UB};

template <typename _Tp> class Pave;
template <typename _Tp> class Pave_node;
template <typename _Tp> class Maze;

template <typename _Tp=ibex::IntervalVector>
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
    const bool is_equal(const SmartSubPaving<_Tp>& g) const;

    /**
     * @brief Return the i-th Pave of the bisectable pave list
     * @param i
     * @return
     */
    const Pave<_Tp>* operator[](std::size_t i) const;

    /**
     * @brief Return the coordinates of the SmartSubPaving
     * @return an interval vector of the coordinates
     */
    const ibex::IntervalVector &get_position() const;

    /**
     * @brief Return the list of Paves of the SmartSubPaving
     * @return A list of pointer to the paves
     */
    const std::vector<Pave<_Tp> *>& get_paves() const;

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
    void add_paves(Pave<_Tp> *p);

    /**
     * @brief Get the number of Paves in the bisectable and not_bisectable list
     * @return
     */
    const size_t size() const;

    /**
     * @brief size active
     * @return
     */
    const size_t size_active() const;

    /**
     * @brief Get the list of all paves that are not bisectable
     * @return
     */
    const std::vector<Pave<_Tp> *>& get_paves_not_bisectable() const;

    /**
     * @brief Getter to the root of the tree pave node
     * @return
     */
    Pave_node<_Tp> *get_tree() const;

    /**
     * @brief Getter to the vector of mazes
     * @return
     */
    const std::vector<Maze<_Tp> *>& get_mazes() const;

    /**
     * @brief Add a new maze to the vector of mazes
     * @param maze
     * @param room
     */
    void add_maze(Maze<_Tp> * maze);

    /**
     * @brief Get info of all room inside position
     * @param maze
     * @param position
     * @return
     */
    void get_room_info(Maze<_Tp>* maze, const ibex::IntervalVector &position, std::vector<Pave<_Tp> *> &pave_list) const;

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

    /**
     * @brief Set the ratio choice between dimension when bisecting
     * @param ratio_bisection
     */
    void set_ratio_bisection(const std::vector<double> &ratio_bisection);

    /**
     * @brief Enable a bisection strategy on a specific dimension
     * @param dim
     * @param enable
     */
    void set_enable_bisection_strategy(const int &dim, const BISECTION_STRATEGY &bisection_type);

    /**
     * @brief Set the size of slice for bisection strategy [lb(), lb()+val] ; [lb()+val, ub()]
     * @param dim
     * @param val
     */
    void set_bisection_strategy_slice(const int &dim, const double &val);

protected:
    /** Class Variable **/
    mutable ibex::IntervalVector    m_position; // SmartSubPaving coordinates
    std::vector<Pave<_Tp>*>         m_paves; // Paves of the SmartSubPaving
    std::vector<Pave<_Tp>*>         m_paves_not_bisectable; // Paves of the SmartSubPaving
    mutable unsigned char           m_dim = 0; // Dimension of the space
    mutable Pave_node<_Tp>*         m_tree = nullptr; // Root of the pave node tree

    std::vector<Maze<_Tp>*>         m_mazes;

    std::vector<double> m_ratio_dimension;
    std::vector<double> m_limit_bisection;

    std::vector<BISECTION_STRATEGY> m_bisection_strategy;
    std::vector<double> m_bisection_strategy_slice;

    omp_lock_t m_write_add_pave;
};

    /**
     * @brief Overloading of the operator <<
     * @param stream
     * @param SmartSubPaving
     * @return
     */
template<typename _Tp>
    std::ostream& operator<< (std::ostream& stream, const SmartSubPaving<_Tp>& g);

}

/// ***** Inline functions *****///

namespace invariant{
template<typename _Tp>
inline Pave_node<_Tp>* SmartSubPaving<_Tp>::get_tree() const{
    return m_tree;
}

template<typename _Tp>
inline const std::vector<Pave<_Tp> *> &SmartSubPaving<_Tp>::get_paves_not_bisectable() const{
    return m_paves_not_bisectable;
}

template<typename _Tp>
inline const size_t SmartSubPaving<_Tp>::size() const{
    return m_paves.size() + m_paves_not_bisectable.size();
}

template<typename _Tp>
inline const size_t SmartSubPaving<_Tp>::size_active() const{
    return m_paves.size();
}

template<typename _Tp>
inline const Pave<_Tp>* SmartSubPaving<_Tp>::operator[](std::size_t i) const{
    return m_paves[i];
}

template<typename _Tp>
inline const unsigned char& SmartSubPaving<_Tp>::dim() const{
    return m_dim;
}

template<typename _Tp>
inline std::ostream& operator<< (std::ostream& stream, const SmartSubPaving<_Tp>& g) {
    stream << "GRAPH : " << g.get_position() << " - " << g.get_paves().size() << " paves";
    return stream;
}
template<typename _Tp>
inline const ibex::IntervalVector& SmartSubPaving<_Tp>::get_position() const{
    return m_position;
}

template<typename _Tp>
inline const std::vector<Pave<_Tp> *> &SmartSubPaving<_Tp>::get_paves() const{
    return m_paves;
}

template<typename _Tp>
inline void SmartSubPaving<_Tp>::add_paves(Pave<_Tp> *p){
    omp_set_lock(&m_write_add_pave);
    m_paves.push_back(p);
    omp_unset_lock(&m_write_add_pave);
}

template<typename _Tp>
inline const std::vector<Maze<_Tp> *>& SmartSubPaving<_Tp>::get_mazes() const{
    return m_mazes;
}

template<typename _Tp>
inline void SmartSubPaving<_Tp>::add_maze(Maze<_Tp> * maze){
    m_mazes.push_back(maze);
}

template<typename _Tp>
inline void SmartSubPaving<_Tp>::set_limit_bisection(const std::vector<double> &limit_bisection){
    if(limit_bisection.size() != m_dim)
        throw std::runtime_error("in [paving.cpp/set_limit_bisection] dimensions doesn't match");
    else
        m_limit_bisection = limit_bisection;
}

template<typename _Tp>
inline void SmartSubPaving<_Tp>::set_ratio_bisection(const std::vector<double> &ratio_bisection){
    if(ratio_bisection.size() != m_dim)
        throw std::runtime_error("in [paving.cpp/set_limit_bisection] dimensions doesn't match");
    else
        m_ratio_dimension = ratio_bisection;
}

template<typename _Tp>
inline void SmartSubPaving<_Tp>::set_enable_bisection_strategy(const int &dim, const BISECTION_STRATEGY &bisection_type){
    m_bisection_strategy[dim] = bisection_type;
}

template<typename _Tp>
inline void SmartSubPaving<_Tp>::set_bisection_strategy_slice(const int &dim, const double &val){
    m_bisection_strategy_slice[dim] = val;
}

}

#include "smartSubPaving.tpp"
#endif // GRAPH_H
