#include "doorouter.h"

namespace invariant {

DoorOuter::DoorOuter(Face *face, Room *room):
    Door(face, room)
{
    m_input_private.set_empty();
    m_output_private.set_empty();
    m_input_public.set_empty();
    m_output_public.set_empty();
}

}
