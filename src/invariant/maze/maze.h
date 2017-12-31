#ifndef MAZE_H
#define MAZE_H

#include <ibex_IntervalVector.h>
#include <deque>
#include <omp.h>
#include <iostream>

#include "../definition/dynamics.h"
#include "../definition/domain.h"
#include "room.h"
#include "../smartSubPaving/smartSubPaving.h"
#include "pave.h"

namespace invariant {

template <typename _Tp> class Maze;
using MazePPL = Maze<Parma_Polyhedra_Library::C_Polyhedron>;
using MazeIBEX = Maze<ibex::IntervalVector>;

class Dynamics; // declared only for friendship
template <typename _Tp> class Room;
template <typename _Tp> class Domain;
template <typename _Tp> class SmartSubPaving;
template <typename _Tp> class Pave;
template <typename _Tp> class Face;
template <typename _Tp> class Door;

template <typename _Tp=ibex::IntervalVector>
class Maze
{
public:
    /**
     * @brief Constructor of a Maze
     * @param g
     * @param f_vect
     * @param type of operation (forward, backward or both)
     */
    Maze(invariant::Domain<_Tp> *domain, Dynamics *dynamics);

    /**
     * @brief Maze destructor
     */
    ~Maze();

    /**
     * @brief Getter to the Domain
     * @return
     */
    Domain<_Tp> * get_domain() const;

    /**
     * @brief Getter to the SmartSubPaving
     * @return
     */
    SmartSubPaving<_Tp> * get_subpaving() const;

    /**
     * @brief Getter to the dynamics
     * @return
     */
    Dynamics *get_dynamics() const;

    /**
     * @brief Contract the Maze
     * @return the number of contractions
     */
    int contract(size_t nb_operations=0);

    /**
     * @brief Contract the Maze by intersecting with other domain mazes
     */
//    void contract_inter(Maze *maze_n);

    /**
     * @brief Add a Room to the deque BUT DO NOT CHECK if already in
     * (to short locking the deque)
     * @param r
     */
    void add_to_deque(Room<_Tp> *r);

    /**
     * @brief Add a list of rooms to the deque (check if already in)
     * @param list_rooms
     */
    void add_rooms(const std::vector<Room<_Tp> *> &list_rooms);

    /**
     * @brief Return true if some trajectory can escape from the search space
     * @return
     */
    bool is_escape_trajectories();

    /**
     * @brief Return true if all rooms of the maze are empty
     * @return
     */
    bool is_empty();

    /**
     * @brief get the bounding box
     * @return
     */
    ibex::IntervalVector get_bounding_box();

    /**
     * @brief compute_vector_field
     */
    void compute_vector_field();

    /**
     * @brief get widening limit
     * @return
     */
    size_t get_widening_limit() const;

    /**
     * @brief set widening limit
     * @param limit
     */
    void set_widening_limit(size_t limit);

    /**
     * @brief Set nb_operation to 0
     */
    void reset_nb_operations();

private:
    invariant::Domain<_Tp> *    m_domain = NULL;
    SmartSubPaving<_Tp>  *    m_subpaving = NULL; // SmartSubPaving associated with this maze
    Dynamics *  m_dynamics = NULL;

    std::deque<Room<_Tp> *> m_deque_rooms;

    omp_lock_t  m_deque_access;

    bool    m_espace_trajectories = true;

    bool    m_contract_once = false;

    bool m_empty = false;
    int m_contraction_step = 0;

    size_t m_nb_operations=0;

    size_t m_widening_limit=1000;
};
}

namespace invariant{

template <typename _Tp>
inline void Maze<_Tp>::set_widening_limit(size_t limit){
    m_widening_limit = limit;
}

template <typename _Tp>
inline size_t Maze<_Tp>::get_widening_limit() const{
    return m_widening_limit;
}

template <typename _Tp>
inline Domain<_Tp> * Maze<_Tp>::get_domain() const{
    return m_domain;
}

template <typename _Tp>
inline Dynamics * Maze<_Tp>::get_dynamics() const{
    return m_dynamics;
}

template <typename _Tp>
inline SmartSubPaving<_Tp> * Maze<_Tp>::get_subpaving() const{
    return m_subpaving;
}

template <typename _Tp>
inline void Maze<_Tp>::add_to_deque(Room<_Tp> *r){
    omp_set_lock(&m_deque_access);
    m_deque_rooms.push_back(r);
    omp_unset_lock(&m_deque_access);
}

}

#include "maze.tpp"

#endif // MAZE_H
