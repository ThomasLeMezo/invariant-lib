#ifndef MAZE_H
#define MAZE_H

#include <ibex.h>
#include <deque>
#include <omp.h>

#include "../definition/dynamics.h"
#include "../definition/domain.h"
#include "room.h"

namespace invariant {
class Graph; // declared only for friendship
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
     * @brief Getter to the Graph
     * @return
     */
    Graph * get_graph() const;

    /**
     * @brief Getter to the dynamics
     * @return
     */
    Dynamics *get_dynamics() const;

    /**
     * @brief Contract the Maze
     * @return the number of contractions
     */
    int contract();

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

private:
    invariant::Domain *    m_domain = NULL;
    Graph  *    m_graph = NULL; // Graph associated with this maze
    Dynamics *  m_dynamics = NULL;

    std::deque<Room *> m_deque_rooms;

    omp_lock_t  m_deque_access;

};
}

namespace invariant{

inline Domain * Maze::get_domain() const{
    return m_domain;
}

inline Dynamics * Maze::get_dynamics() const{
    return m_dynamics;
}

inline Graph * Maze::get_graph() const{
    return m_graph;
}

inline void Maze::add_to_deque(Room *r){
    omp_set_lock(&m_deque_access);
    m_deque_rooms.push_back(r);
    omp_unset_lock(&m_deque_access);
}

}
#endif // MAZE_H
