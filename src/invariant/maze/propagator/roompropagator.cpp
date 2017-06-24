#include "roompropagator.h"
#include "doorpropagator.h"

namespace invariant {

RoomPropagator::RoomPropagator(Pave *p, Maze *m, Dynamics *dynamics) : Room(p, m, dynamics)
{

    int dim = m_pave->get_dim();
    for(int face=0; face<dim; face++){
        for(int sens=0; sens < 2; sens++){
            Door *d = new DoorPropagator(m_pave->get_faces()[face][sens], this);
            Face *f = m_pave->get_faces()[face][sens];
            f->add_door(d);
        }
    }
}

}
