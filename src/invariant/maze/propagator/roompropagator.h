#ifndef ROOMPROPAGATOR_H
#define ROOMPROPAGATOR_H

#include "room.h"

namespace invariant {

class RoomPropagator : public Room
{
public:
    RoomPropagator(Pave *p, Maze *m, Dynamics *dynamics);
};

}
#endif // ROOMPROPAGATOR_H
