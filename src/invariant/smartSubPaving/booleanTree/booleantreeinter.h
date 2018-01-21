#ifndef BISECTIONTREEINTER_H
#define BISECTIONTREEINTER_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "booleantree.h"
#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _Tp=ibex::IntervalVector>
class BooleanTreeInter : public BooleanTree<_Tp>
{
public:
    BooleanTreeInter(const std::vector<Maze<_Tp>*> &maze_list):BooleanTree<_Tp>(maze_list){}
    BooleanTreeInter(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B):BooleanTree<_Tp>(maze_A, maze_B){}
    BooleanTreeInter(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B, Maze<_Tp>* maze_C):BooleanTree<_Tp>(maze_A, maze_B, maze_C){}

    BooleanTreeInter(std::vector<BooleanTree<_Tp>*> bisectionTree_list):BooleanTree<_Tp>(bisectionTree_list){}
    BooleanTreeInter(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B):BooleanTree<_Tp>(bisectionTree_A, bisectionTree_B){}
    BooleanTreeInter(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B, BooleanTree<_Tp>* bisectionTree_C):BooleanTree<_Tp>(bisectionTree_A, bisectionTree_B, bisectionTree_C){}

    BooleanTreeInter(std::vector<Maze<_Tp>*> maze_list, std::vector<BooleanTree<_Tp>*> bisectionTree_list):BooleanTree<_Tp>(maze_list, bisectionTree_list){}
    BooleanTreeInter(Maze<_Tp>* maze_A, BooleanTree<_Tp>* bisectionTree_A):BooleanTree<_Tp>(maze_A, bisectionTree_A){}

    bool eval_bisection(Pave<_Tp> *pave);

    bool eval_set_empty(Pave<_Tp> *pave);
};

using BooleanTreeInterPPL = BooleanTreeInter<Parma_Polyhedra_Library::C_Polyhedron>;
using BooleanTreeInterIBEX = BooleanTreeInter<ibex::IntervalVector>;

}

#include "booleantreeinter.tpp"

#endif // BISECTIONTREEINTER_H
