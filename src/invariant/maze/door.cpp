#include "door.h"

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

}
