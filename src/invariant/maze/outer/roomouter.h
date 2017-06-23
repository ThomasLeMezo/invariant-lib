#ifndef ROOMOUTER_H
#define ROOMOUTER_H

#include "room.h"

namespace invariant {

class RoomOuter : public Room
{
public:
    RoomOuter(Pave *p, Maze *m, Dynamics *dynamics);
};

}
#endif // ROOMOUTER_H
