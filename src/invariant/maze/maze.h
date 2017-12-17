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

template <typename _Tp, typename _V> class Maze;
using MazePPL = Maze<Parma_Polyhedra_Library::C_Polyhedron, Parma_Polyhedra_Library::Generator_System>;
using MazeIBEX = Maze<ibex::IntervalVector, ibex::IntervalVector>;

class Dynamics; // declared only for friendship
template <typename _Tp, typename _V> class Room;
template <typename _Tp, typename _V> class Domain;
template <typename _Tp, typename _V> class SmartSubPaving;
template <typename _Tp, typename _V> class Pave;
template <typename _Tp, typename _V> class Face;
template <typename _Tp, typename _V> class Door;

template <typename _Tp=ibex::IntervalVector, typename _V=ibex::IntervalVector>
class Maze
{
public:
    /**
     * @brief Constructor of a Maze
     * @param g
     * @param f_vect
     * @param type of operation (forward, backward or both)
     */
    Maze(invariant::Domain<_Tp, _V> *domain, Dynamics *dynamics);

    /**
     * @brief Maze destructor
     */
    ~Maze();

    /**
     * @brief Getter to the Domain
     * @return
     */
    Domain<_Tp, _V> * get_domain() const;

    /**
     * @brief Getter to the SmartSubPaving
     * @return
     */
    SmartSubPaving<_Tp, _V> * get_subpaving() const;

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
    void add_to_deque(Room<_Tp, _V> *r);

    /**
     * @brief Add a list of rooms to the deque (check if already in)
     * @param list_rooms
     */
    void add_rooms(const std::vector<Room<_Tp, _V> *> &list_rooms);

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

private:
    invariant::Domain<_Tp, _V> *    m_domain = NULL;
    SmartSubPaving<_Tp, _V>  *    m_subpaving = NULL; // SmartSubPaving associated with this maze
    Dynamics *  m_dynamics = NULL;

    std::deque<Room<_Tp, _V> *> m_deque_rooms;

    omp_lock_t  m_deque_access;

    bool    m_espace_trajectories = true;

    bool    m_contract_once = false;

    bool m_empty = false;
    int m_contraction_step = 0;

    size_t m_nb_operations=0;

};
}

namespace invariant{

template <typename _Tp, typename _V>
inline Domain<_Tp, _V> * Maze<_Tp, _V>::get_domain() const{
    return m_domain;
}

template <typename _Tp, typename _V>
inline Dynamics * Maze<_Tp, _V>::get_dynamics() const{
    return m_dynamics;
}

template <typename _Tp, typename _V>
inline SmartSubPaving<_Tp, _V> * Maze<_Tp, _V>::get_subpaving() const{
    return m_subpaving;
}

template <typename _Tp, typename _V>
inline void Maze<_Tp, _V>::add_to_deque(Room<_Tp, _V> *r){
    omp_set_lock(&m_deque_access);
    m_deque_rooms.push_back(r);
    omp_unset_lock(&m_deque_access);
}

}

#include "maze.tpp"

#endif // MAZE_H
