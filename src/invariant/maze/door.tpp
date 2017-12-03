#include "door.h"
#include <iostream>
#include "omp.h"

#include "../definition/domain.h"

#include "../smartSubPaving/face.h"
#include "../smartSubPaving/pave.h"
#include "room.h"
#include "maze.h"

namespace invariant {

//template <typename _Tp>
//Door<_Tp>::Door(invariant::Face<_Tp> *face, invariant::Room<_Tp> *room):
//    m_input_public(0),
//    m_output_public(0)
//{
//    m_input_private = new _Tp(face->get_position());
//    m_output_private = new _Tp(face->get_position());

//    m_face = face;
//    m_room = room;
//    omp_init_lock(&m_lock_read);
//}

template <typename _Tp>
Door<_Tp>::~Door(){
    omp_destroy_lock(&m_lock_read);
    if(m_input_private != NULL)
        delete(m_input_private);
    if(m_output_private != NULL)
        delete(m_output_private);
}

template <typename _Tp>
void Door<_Tp>::set_removed(){
    delete(m_output_private);
    delete(m_input_private);

    m_output_private = NULL;
    m_input_private = NULL;
}

template <typename _Tp>
void Door<_Tp>::synchronize(){
    omp_set_lock(&m_lock_read);
    m_input_public = *m_input_private;
    m_output_public = *m_output_private;
    omp_unset_lock(&m_lock_read);
}

template <typename _Tp>
bool Door<_Tp>::analyze_change(std::vector<Room<_Tp> *>&list_rooms){
    if((*m_input_private) != get_input()
            || (*m_output_private) != get_output()){ // operator != is generic for iv & polyhedron
        std::vector<Face<_Tp> *> l_face = m_face->get_neighbors();
        for(Face<_Tp>* f:l_face)
            list_rooms.push_back(f->get_pave()->get_rooms()[m_room->get_maze()]);
        return true;
    }
    return false;
}

template <typename _Tp>
const _Tp Door<_Tp>::get_hull() const{
    return m_input_public | m_output_public;
}

template <typename _Tp>
void Door<_Tp>::set_full_private_output(){
    (*m_output_private) = m_face->get_position_typed();
}

template <typename _Tp>
void Door<_Tp>::set_full_private_input(){
   (*m_input_private) = m_face->get_position_typed();
}

template <typename _Tp>
void Door<_Tp>::set_full_private(){
    (*m_output_private) = m_face->get_position_typed();
    (*m_input_private) = m_face->get_position_typed();
}

template <typename _Tp>
void Door<_Tp>::set_full_possible_private(){
    for(size_t i=0; i<m_possible_in.size(); i++){
        if(m_possible_in[i])
            (*m_input_private) = m_face->get_position_typed();
        if(m_possible_out[i])
            (*m_output_private) = m_face->get_position_typed();
    }
}

template <typename _Tp>
bool Door<_Tp>::is_full() const{
    if(m_input_public == m_face->get_position_typed() && m_output_public == m_face->get_position_typed())
        return true;
    else
        return false;
}

template <typename _Tp>
bool Door<_Tp>::is_full_union() const{
    if((m_input_public | m_output_public) == m_face->get_position())
        return true;
    else
        return false;
}

template <typename _Tp>
bool Door<_Tp>::contract_continuity_private(){
    DYNAMICS_SENS dynamics_sens = m_room->get_maze()->get_dynamics()->get_sens();
    DOMAIN_INITIALIZATION domain_init = m_room->get_maze()->get_domain()->get_init();

    bool change = false;

    if(dynamics_sens == FWD || dynamics_sens == FWD_BWD){
        _Tp door_input = get_empty_door_container<_Tp>(m_face->get_pave()->get_dim());
        for(Face<_Tp>* f:m_face->get_neighbors()){
            Door<_Tp> *d = f->get_doors()[m_room->get_maze()];
            door_input |= (d->get_output() & m_face->get_position_typed());
        }
        if(door_input != *m_input_private){
            change = true;
            if(domain_init == FULL_DOOR)
                (*m_input_private) &= door_input;
            else if(domain_init == FULL_WALL)
                (*m_input_private) |= door_input;
        }
    }

    if(dynamics_sens == BWD || dynamics_sens == FWD_BWD){
        _Tp door_output = get_empty_door_container<_Tp>(m_face->get_pave()->get_dim());
        for(Face<_Tp>* f:m_face->get_neighbors()){
            Door<_Tp> *d = f->get_doors()[m_room->get_maze()];
            door_output |= (d->get_input() & m_face->get_position_typed());
        }
        if(door_output != (*m_output_private)){
            change = true;
            if(domain_init == FULL_DOOR)
                (*m_output_private) &= door_output;
            else if(domain_init == FULL_WALL)
                (*m_output_private) |= door_output;
        }
    }
    return change;
}

/// ******************  Sepcialized ****************** ///



}
