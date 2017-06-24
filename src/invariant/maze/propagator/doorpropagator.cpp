#include "doorpropagator.h"

using namespace ibex;
using namespace std;

namespace invariant {

DoorPropagator::DoorPropagator(Face *face, Room *room):
    Door(face, room)
{
    m_input_private.set_empty();
    m_output_private.set_empty();
    m_input_public.set_empty();
    m_output_public.set_empty();
}

bool DoorPropagator::contract_continuity_private(){
    bool change = false;
    IntervalVector door_input = ibex::IntervalVector(m_input_private.size(), Interval::EMPTY_SET);
    IntervalVector door_output = ibex::IntervalVector(m_output_private.size(), Interval::EMPTY_SET);
    for(Face* f:m_face->get_neighbors()){
        Door *d = f->get_doors()[m_room->get_maze()];
        door_input |= d->get_output();
        door_output |= d->get_input();
    }

    if(door_input != m_input_private
            || door_output != m_output_private){
        change = true;
        m_input_private |= door_input;
        m_output_private |= door_output;
    }

    return change;
}

}

