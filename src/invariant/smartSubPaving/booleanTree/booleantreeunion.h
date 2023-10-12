#ifndef BISECTIONTREEUNION_H
#define BISECTIONTREEUNION_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "booleantree.h"
#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _TpR=ibex::IntervalVector,typename _TpF=ibex::IntervalVector,typename _TpD=ibex::IntervalVector>
class BooleanTreeUnion : public BooleanTree<_TpR,_TpF,_TpD>
{
public:

    BooleanTreeUnion(const std::vector<Maze<_TpR,_TpF,_TpD>*> &maze_list):BooleanTree<_TpR,_TpF,_TpD>(maze_list){}
    BooleanTreeUnion(Maze<_TpR,_TpF,_TpD>* maze_A, Maze<_TpR,_TpF,_TpD>* maze_B):BooleanTree<_TpR,_TpF,_TpD>(maze_A, maze_B){}
    BooleanTreeUnion(Maze<_TpR,_TpF,_TpD>* maze_A, Maze<_TpR,_TpF,_TpD>* maze_B, Maze<_TpR,_TpF,_TpD>* maze_C):BooleanTree<_TpR,_TpF,_TpD>(maze_A, maze_B, maze_C){}

    BooleanTreeUnion(const std::vector<BooleanTree<_TpR,_TpF,_TpD>*> &bisectionTree_list):BooleanTree<_TpR,_TpF,_TpD>(bisectionTree_list){}
    BooleanTreeUnion(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_B):BooleanTree<_TpR,_TpF,_TpD>(bisectionTree_A, bisectionTree_B){}
    BooleanTreeUnion(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_B, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_C):BooleanTree<_TpR,_TpF,_TpD>(bisectionTree_A, bisectionTree_B, bisectionTree_C){}

    BooleanTreeUnion(const std::vector<Maze<_TpR,_TpF,_TpD>*> &maze_list, const std::vector<BooleanTree<_TpR,_TpF,_TpD>*> &bisectionTree_list):BooleanTree<_TpR,_TpF,_TpD>(maze_list, bisectionTree_list){}
    BooleanTreeUnion(Maze<_TpR,_TpF,_TpD>* maze_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A):BooleanTree<_TpR,_TpF,_TpD>(maze_A, bisectionTree_A){}

    bool eval_bisection(Pave<_TpR,_TpF,_TpD> *pave);

    bool eval_set_empty(Pave<_TpR,_TpF,_TpD> *pave);
};

using BooleanTreeUnionPPL = BooleanTreeUnion<Parma_Polyhedra_Library::C_Polyhedron>;
using BooleanTreeUnionIBEX = BooleanTreeUnion<ibex::IntervalVector>;

}

#include "booleantreeunion.tpp"

#endif // BISECTIONTREEUNION_H
