#include "bisectiontreeunion.h"

#include "room.h"

namespace invariant {

template<typename _Tp>
bool BisectionTreeUnion<_Tp>::eval(Pave<_Tp> *pave)
{
    bool result = false;
    for(Maze<_Tp>* maze:this->m_maze_list){
        Room<_Tp>* r = pave->get_rooms()[maze];
        result |= r->request_bisection();
    }

    for(BisectionTree<_Tp>* bisectTree:this->m_children_list){
        result |= bisectTree->eval(pave);
    }
    return result;
}

}
