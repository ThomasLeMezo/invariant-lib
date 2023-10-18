#include "booleantreenot.h"
#include "room.h"

namespace invariant {

template<typename _TpR, typename _TpF, typename _TpD>
bool BooleanTreeNot<_TpR,_TpF,_TpD>::eval_bisection(Pave<_TpR,_TpF,_TpD> *pave){
    if(!this->m_maze_list.empty()){
        Maze<_TpR,_TpF,_TpD> *maze = this->m_maze_list[0];
        return !pave->get_rooms()[maze]->request_bisection();
    }

    if(!this->m_children_list.empty()){
        BooleanTree<_TpR,_TpF,_TpD> *bisectTree = this->m_children_list[0];
        return !bisectTree->eval_bisection(pave);
    }

    return true;
}

template<typename _TpR, typename _TpF, typename _TpD>
bool BooleanTreeNot<_TpR,_TpF,_TpD>::eval_set_empty(Pave<_TpR,_TpF,_TpD> *pave)
{
    if(!this->m_maze_list.empty()){
        Maze<_TpR,_TpF,_TpD> *maze = this->m_maze_list[0];
        return !pave->get_rooms()[maze]->is_empty();
    }

    if(!this->m_children_list.empty()){
        BooleanTree<_TpR,_TpF,_TpD> *bisectTree = this->m_children_list[0];
        return !bisectTree->eval_set_empty(pave);
    }
    return true;
}

}
