#ifndef MAZE_H
#define MAZE_H

#include <ibex/ibex_IntervalVector.h>
#include <deque>
#include <omp.h>

#include "../definition/dynamics.h"
#include "../definition/domain.h"
#include "room.h"

namespace invariant {

class SmartSubPaving; // declared only for friendship
class Domain; // declared only for friendship
class Dynamics; // declared only for friendship
class Room; // declared only for friendship
class Maze
{
public:
    /**
     * @brief Constructor of a Maze
     * @param g
     * @param f_vect
     * @param type of operation (forward, backward or both)
     */
    Maze(invariant::Domain *domain, Dynamics *dynamics);

    /**
     * @brief Maze destructor
     */
    ~Maze();

    /**
     * @brief Getter to the Domain
     * @return
     */
    Domain * get_domain() const;

    /**
     * @brief Getter to the SmartSubPaving
     * @return
     */
    SmartSubPaving * get_subpaving() const;

    /**
     * @brief Getter to the dynamics
     * @return
     */
    Dynamics *get_dynamics() const;

    /**
     * @brief Initialize the Maze by applying only a domain contractor
     */
    void init();

    /**
     * @brief Contract the Maze
     * @return the number of contractions
     */
    int contract();

    /**
     * @brief Contract the Maze by intersecting with other domain mazes
     */
//    void contract_inter(Maze *maze_n);

    /**
     * @brief Add a Room to the deque BUT DO NOT CHECK if already in
     * (to short locking the deque)
     * @param r
     */
    void add_to_deque(Room *r);

    /**
     * @brief Add a list of rooms to the deque (check if already in)
     * @param list_rooms
     */
    void add_rooms(const std::vector<Room *> &list_rooms);

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
    invariant::Domain *    m_domain = NULL;
    SmartSubPaving  *    m_subpaving = NULL; // SmartSubPaving associated with this maze
    Dynamics *  m_dynamics = NULL;

    std::deque<Room *> m_deque_rooms;

    omp_lock_t  m_deque_access;

    bool    m_espace_trajectories = true;

    bool    m_contract_once = false;

    bool m_empty = false;

};
}

namespace invariant{

inline Domain * Maze::get_domain() const{
    return m_domain;
}

inline Dynamics * Maze::get_dynamics() const{
    return m_dynamics;
}

inline SmartSubPaving * Maze::get_subpaving() const{
    return m_subpaving;
}

inline void Maze::add_to_deque(Room *r){
    omp_set_lock(&m_deque_access);
    m_deque_rooms.push_back(r);
    omp_unset_lock(&m_deque_access);
}

}
#endif // MAZE_H
