#include "booleantreeunion.h"

#include "room.h"

namespace invariant {

template<typename _TpR, typename _TpF, typename _TpD>
bool BooleanTreeUnion<_TpR,_TpF,_TpD>::eval_bisection(Pave<_TpR,_TpF,_TpD> *pave)
{
    bool result = false;
    for(Maze<_TpR,_TpF,_TpD>* maze:this->m_maze_list){
        Room<_TpR,_TpF,_TpD>* r = pave->get_rooms()[maze];
        result |= r->request_bisection();
    }

    for(BooleanTree<_TpR,_TpF,_TpD>* bisectTree:this->m_children_list){
        result |= bisectTree->eval_bisection(pave);
    }
    return result;
}

template<typename _TpR, typename _TpF, typename _TpD>
bool BooleanTreeUnion<_TpR,_TpF,_TpD>::eval_set_empty(Pave<_TpR,_TpF,_TpD> *pave)
{
    bool result = false;
    for(Maze<_TpR,_TpF,_TpD>* maze:this->m_maze_list){
        Room<_TpR,_TpF,_TpD>* r = pave->get_rooms()[maze];
        result |= r->is_empty();
    }

    for(BooleanTree<_TpR,_TpF,_TpD>* bisectTree:this->m_children_list){
        result |= bisectTree->eval_set_empty(pave);
    }
    return result;
}

}
