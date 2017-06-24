#include "mazepropagator.h"
#include "roompropagator.h"

namespace invariant {

MazePropagator::MazePropagator(invariant::Domain *domain, invariant::Dynamics *dynamics, MazeType maze_type):
    Maze(domain, dynamics, maze_type)
{
    Graph *g = domain->get_graph();
    for(Pave*p:g->get_paves()){
        RoomPropagator *r = new RoomPropagator(p, this, dynamics);
        p->add_room(r);
    }
}

}
