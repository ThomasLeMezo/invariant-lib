#ifndef MAZE_H
#define MAZE_H

#include <ibex.h>
#include "dynamics.h"
#include "domain.h"

namespace invariant {
class Graph; // declared only for friendship
class Domain; // declared only for friendship
class Dynamics; // declared only for friendship
class Maze
{
public:
    /**
     * @brief Constructor of a Maze
     * @param g
     * @param f_vect
     */
    Maze(Domain *domain, Dynamics *dynamics);

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
     * @brief Contract the maze
     */
    void contract();

private:
    Domain *    m_domain = NULL;
    Graph  *    m_graph = NULL; // Graph associated with this maze
    Dynamics *  m_dynamics = NULL;

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

}
#endif // MAZE_H
