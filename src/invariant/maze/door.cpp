#include "door.h"

using namespace ibex;
namespace invariant {

Door::Door(Face *face, Room *room):
    m_input_private(face->get_position()), m_output_private(face->get_position()),
    m_input_public(face->get_position()), m_output_public(face->get_position())
{
    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);
}

Door::~Door(){
    omp_destroy_lock(&m_lock_read);
}

void Door::synchronize(){
    omp_set_lock(&m_lock_read);
    m_input_public = m_input_private;
    m_output_public = m_output_private;
    omp_unset_lock(&m_lock_read);
}

void Door::contract_continuity_private(){
    IntervalVector door_input = ibex::IntervalVector::empty(m_input_private.size());
    IntervalVector door_output = ibex::IntervalVector::empty(m_input_private.size());
    for(Face* f:m_face->get_neighbors()){
        Door *d = f->get_doors()[m_room->get_maze()];
        door_input |= d->get_input();
        door_output |= d->get_output();
    }

    m_input_private &= door_input;
    m_output_private &= door_output;
}
}
