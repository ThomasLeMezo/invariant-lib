#ifndef BISECTIONTREEUNION_H
#define BISECTIONTREEUNION_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "booleantree.h"
#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _Tp=ibex::IntervalVector>
class BooleanTreeUnion : public BooleanTree<_Tp>
{
public:

    BooleanTreeUnion(const std::vector<Maze<_Tp>*> &maze_list):BooleanTree<_Tp>(maze_list){}
    BooleanTreeUnion(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B):BooleanTree<_Tp>(maze_A, maze_B){}
    BooleanTreeUnion(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B, Maze<_Tp>* maze_C):BooleanTree<_Tp>(maze_A, maze_B, maze_C){}

    BooleanTreeUnion(std::vector<BooleanTree<_Tp>*> bisectionTree_list):BooleanTree<_Tp>(bisectionTree_list){}
    BooleanTreeUnion(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B):BooleanTree<_Tp>(bisectionTree_A, bisectionTree_B){}
    BooleanTreeUnion(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B, BooleanTree<_Tp>* bisectionTree_C):BooleanTree<_Tp>(bisectionTree_A, bisectionTree_B, bisectionTree_C){}

    BooleanTreeUnion(std::vector<Maze<_Tp>*> maze_list, std::vector<BooleanTree<_Tp>*> bisectionTree_list):BooleanTree<_Tp>(maze_list, bisectionTree_list){}
    BooleanTreeUnion(Maze<_Tp>* maze_A, BooleanTree<_Tp>* bisectionTree_A):BooleanTree<_Tp>(maze_A, bisectionTree_A){}

    bool eval_bisection(Pave<_Tp> *pave);

    bool eval_set_empty(Pave<_Tp> *pave);
};

using BooleanTreeUnionPPL = BooleanTreeUnion<Parma_Polyhedra_Library::C_Polyhedron>;
using BooleanTreeUnionIBEX = BooleanTreeUnion<ibex::IntervalVector>;

}

#include "booleantreeunion.tpp"

#endif // BISECTIONTREEUNION_H
