#include "maze.h"

namespace invariant {

Maze::Maze(Domain *domain, Dynamics *dynamics)
{
    m_domain = domain;
    m_dynamics = dynamics;
}

}
