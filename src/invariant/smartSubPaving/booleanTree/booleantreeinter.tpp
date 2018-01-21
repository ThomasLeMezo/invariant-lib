#include "booleantreeinter.h"
#include "room.h"

namespace invariant {

template<typename _Tp>
bool BooleanTreeInter<_Tp>::eval_bisection(Pave<_Tp> *pave)
{
    bool result = true;
    for(Maze<_Tp>* maze:this->m_maze_list){
        Room<_Tp>* r = pave->get_rooms()[maze];
        result &= r->request_bisection();
    }

    for(BooleanTree<_Tp>* bisectTree:this->m_children_list){
        result &= bisectTree->eval_bisection(pave);
    }
    return result;
}

template<typename _Tp>
bool BooleanTreeInter<_Tp>::eval_set_empty(Pave<_Tp> *pave)
{
    bool result = true;
    for(Maze<_Tp>* maze:this->m_maze_list){
        Room<_Tp>* r = pave->get_rooms()[maze];
        result &= r->is_empty();
    }

    for(BooleanTree<_Tp>* bisectTree:this->m_children_list){
        result &= bisectTree->eval_set_empty(pave);
    }
    return result;
}

}
