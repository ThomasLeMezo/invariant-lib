#include "booleantreenot.h"
#include "room.h"

namespace invariant {

template<typename _Tp>
bool BooleanTreeNot<_Tp>::eval_bisection(Pave<_Tp> *pave){
    if(!this->m_maze_list.empty()){
        Maze<_Tp> *maze = this->m_maze_list[0];
        return !pave->get_rooms()[maze]->request_bisection();
    }

    if(!this->m_children_list.empty()){
        BooleanTree<_Tp> *bisectTree = this->m_children_list[0];
        return !bisectTree->eval_bisection(pave);
    }

    return true;
}

template<typename _Tp>
bool BooleanTreeNot<_Tp>::eval_set_empty(Pave<_Tp> *pave)
{
    if(!this->m_maze_list.empty()){
        Maze<_Tp> *maze = this->m_maze_list[0];
        return !pave->get_rooms()[maze]->is_empty();
    }

    if(!this->m_children_list.empty()){
        BooleanTree<_Tp> *bisectTree = this->m_children_list[0];
        return !bisectTree->eval_set_empty(pave);
    }
    return true;
}

}
