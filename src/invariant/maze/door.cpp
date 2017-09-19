#include "door.h"
#include <iostream>
#include "omp.h"

using namespace ibex;
using namespace std;
namespace invariant {

Door::Door(Face *face, Room *room):
    m_input_public(face->get_position()), m_output_public(face->get_position())
{
    m_input_private = new IntervalVector(face->get_position());
    m_output_private = new IntervalVector(face->get_position());

    m_face = face;
    m_room = room;
    omp_init_lock(&m_lock_read);

    if(m_room->get_maze()->get_type() == MAZE_WALL){
        m_input_private->set_empty();
        m_output_private->set_empty();
        m_input_public.set_empty();
        m_output_public.set_empty();
    }
}

Door::~Door(){
    omp_destroy_lock(&m_lock_read);
    if(m_input_private != NULL)
        delete(m_input_private);
    if(m_output_private != NULL)
        delete(m_output_private);
}

void Door::set_removed(){
    delete(m_output_private);
    delete(m_input_private);

    m_output_private = NULL;
    m_input_private = NULL;
}

const IntervalVector Door::get_hull() const{
    return m_input_public | m_output_public;
}

void Door::synchronize(){
    omp_set_lock(&m_lock_read);
    m_input_public = *m_input_private;
    m_output_public = *m_output_private;
    omp_unset_lock(&m_lock_read);
}

bool Door::contract_continuity_private(){
    MazeSens sens = m_room->get_maze()->get_sens();
    MazeType type = m_room->get_maze()->get_type();
    bool change = false;

    if(sens == MAZE_FWD || sens == MAZE_FWD_BWD){
        IntervalVector door_input = ibex::IntervalVector(m_input_private->size(), Interval::EMPTY_SET);
        for(Face* f:m_face->get_neighbors()){
            Door *d = f->get_doors()[m_room->get_maze()];
            door_input |= (d->get_output() & m_face->get_position());
        }
        if(door_input != *m_input_private){
            change = true;
            if(type == MAZE_DOOR)
                (*m_input_private) &= door_input;
            else if(type == MAZE_WALL)
                (*m_input_private) |= door_input;
        }
    }

    if(sens == MAZE_BWD || sens == MAZE_FWD_BWD){
        IntervalVector door_output = ibex::IntervalVector(m_output_private->size(), Interval::EMPTY_SET);
        for(Face* f:m_face->get_neighbors()){
            Door *d = f->get_doors()[m_room->get_maze()];
            door_output |= (d->get_input() & m_face->get_position());
        }
        if(door_output != (*m_output_private)){
            change = true;
            if(type == MAZE_DOOR)
                (*m_output_private) &= door_output;
            else if(type == MAZE_WALL)
                (*m_output_private) |= door_output;
        }
    }
    return change;
}

bool Door::analyze_change(vector<Room *>&list_rooms){
    if((*m_input_private) != get_input()
            || (*m_output_private) != get_output()){
        vector<Face *> l_face = m_face->get_neighbors();
        for(Face* f:l_face)
            list_rooms.push_back(f->get_pave()->get_rooms()[m_room->get_maze()]);
        return true;
    }
    return false;
}

void Door::set_full_private_output(){
    (*m_output_private) = m_face->get_position();
}

void Door::set_full_private_input(){
   (*m_input_private) = m_face->get_position();
}

void Door::set_full_private(){
    (*m_output_private) = m_face->get_position();
    (*m_input_private) = m_face->get_position();
}

void Door::set_full_possible_private(){
    for(size_t i=0; i<m_possible_in.size(); i++){
        if(m_possible_in[i])
            (*m_input_private) = m_face->get_position();
        if(m_possible_out[i])
            (*m_output_private) = m_face->get_position();
    }
}

bool Door::is_full(){
    if(m_input_public == m_face->get_position() && m_output_public == m_face->get_position())
        return true;
    else
        return false;
}

}
