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

    if(m_room->get_maze()->get_maze_type() == MAZE_PROPAGATOR){
        m_input_private.set_empty();
        m_output_private.set_empty();
        m_input_public.set_empty();
        m_output_public.set_empty();
    }
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

bool Door::contract_continuity_private(){
    MazeSens sens = m_room->get_maze()->get_maze_sens();
    MazeType type = m_room->get_maze()->get_maze_type();
    bool change = false;

    if(sens == MAZE_FWD || sens == MAZE_FWD_BWD){
        IntervalVector door_input = ibex::IntervalVector(m_input_private.size(), Interval::EMPTY_SET);
        for(Face* f:m_face->get_neighbors()){
            Door *d = f->get_doors()[m_room->get_maze()];
            door_input |= d->get_output();
        }
        if(door_input != m_input_private){
            change = true;
            if(type == MAZE_CONTRACTOR)
                m_input_private &= door_input;
            else if(type == MAZE_PROPAGATOR)
                m_input_private |= door_input;
        }
    }

    if(sens == MAZE_BWD || sens == MAZE_FWD_BWD){
        IntervalVector door_output = ibex::IntervalVector(m_output_private.size(), Interval::EMPTY_SET);
        for(Face* f:m_face->get_neighbors()){
            Door *d = f->get_doors()[m_room->get_maze()];
            door_output |= d->get_input();
        }
        if(door_output != m_output_private){
            change = true;
            if(type == MAZE_CONTRACTOR)
                m_output_private &= door_output;
            else if(type == MAZE_PROPAGATOR)
                m_output_private |= door_output;
        }
    }
    return change;
}

void Door::analyze_change(std::vector<Room *>&list_rooms){
    if(m_input_private != m_input_public
            || m_output_private != m_output_public){
        std::vector<Face *> l_face = m_face->get_neighbors();
        for(Face* f:l_face){
            list_rooms.push_back(f->get_pave()->get_rooms()[m_room->get_maze()]);
        }
    }
}
}
