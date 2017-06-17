#include "room.h"

using namespace ibex;
namespace invariant {

Room::Room(Pave *p, Maze *m, std::vector<ibex::Function*> f_vect)
{
    m_pave = p;
    m_maze = m;
    const IntervalVector position(p->get_position());
    for(Function*f:f_vect){
        IntervalVector vector_field = f->eval_vector(position);
        m_vector_fields.push_back(vector_field);
    }
}

void Room::set_empty_output(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_output();
    }
}

void Room::set_empty_input(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty_input();
    }
}

void Room::set_empty(){
    for(Face *f:m_pave->get_faces_vector()){
        f->get_doors()[m_maze]->set_empty();
    }
}
}
