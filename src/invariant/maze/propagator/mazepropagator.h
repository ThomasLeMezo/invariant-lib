#ifndef MAZEPROPAGATOR_H
#define MAZEPROPAGATOR_H

#include "maze.h"

namespace invariant {

class MazePropagator : public Maze
{
public:
    MazePropagator(invariant::Domain *domain, invariant::Dynamics *dynamics, MazeType maze_type);
};

}

#endif // MAZEPROPAGATOR_H
