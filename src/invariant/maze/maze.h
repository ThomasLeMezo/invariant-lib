#ifndef MAZE_H
#define MAZE_H

#include <ibex.h>
#include <deque>
#include <omp.h>

#include "../definition/dynamics.h"
#include "../definition/domain.h"
#include "room.h"

namespace invariant {

/**
 * @brief The MazeType enum
 * MAZE_FWD : propagation or contraction in the sens of the vector field
 * MAZE_BWD : propagation or contraction in the opposite sens of the vector field
 * MAZE_FWD_BWD : propagation or contraction in both sens
 */
enum MazeSens {MAZE_FWD, MAZE_BWD, MAZE_FWD_BWD};

/**
 * @brief The MazeType enum
 */
enum MazeType {MAZE_CONTRACTOR, MAZE_PROPAGATOR};

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
     * @param type of operation (forward, backward or both)
     */
    Maze(invariant::Domain *domain, Dynamics *dynamics, MazeSens maze_sens =MAZE_FWD_BWD, MazeType maze_type=MAZE_CONTRACTOR);

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

    /**
     * @brief Return the sens of the Maze
     * @return
     */
    MazeSens get_sens() const;

    /**
     * @brief Return the type of the Maze
     * @return
     */
    MazeType get_type() const;

protected:
    invariant::Domain *    m_domain = NULL;
    Graph  *    m_graph = NULL; // Graph associated with this maze
    Dynamics *  m_dynamics = NULL;

    std::deque<Room *> m_deque_rooms;

    omp_lock_t  m_deque_access;

    MazeSens m_maze_sens = MAZE_FWD_BWD;
    MazeType m_maze_type = MAZE_CONTRACTOR;

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

inline MazeSens Maze::get_sens() const{
    return m_maze_sens;
}

inline MazeType Maze::get_type() const{
    return m_maze_type;
}


}
#endif // MAZE_H
