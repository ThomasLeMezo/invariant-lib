#include "mazeouter.h"
#include "roomouter.h"

namespace invariant {

MazeOuter::MazeOuter(invariant::Domain *domain, invariant::Dynamics *dynamics):
    Maze(domain, dynamics)
{
    Graph *g = domain->get_graph();
    for(Pave*p:g->get_paves()){
        RoomOuter *r = new RoomOuter(p, this, dynamics);
        p->add_room(r);
    }
}

}
