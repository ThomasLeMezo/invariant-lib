#ifndef MAZEOUTER_H
#define MAZEOUTER_H

#include "maze.h"

namespace invariant {

class MazeOuter : public Maze
{
public:
    MazeOuter(invariant::Domain *domain, invariant::Dynamics *dynamics);
};

}

#endif // MAZEOUTER_H
