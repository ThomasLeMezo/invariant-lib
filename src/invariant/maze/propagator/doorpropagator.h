#ifndef DOORPROPAGATOR_H
#define DOORPROPAGATOR_H

#include "door.h"

namespace invariant {

class DoorPropagator : public Door
{
public:
    /**
     * @brief DoorPropagator constructor
     * @param face
     * @param room
     */
    DoorPropagator(Face *face, Room *room);

    /**
     * @brief Contract its private door according to neighbors
     */
    bool contract_continuity_private();
};

}

#endif // DOORPROPAGATOR_H
