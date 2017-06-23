#ifndef DOOROUTER_H
#define DOOROUTER_H

#include "door.h"

namespace invariant {

class DoorOuter : public Door
{
public:
    DoorOuter(Face *face, Room *room);
};

}

#endif // DOOROUTER_H
