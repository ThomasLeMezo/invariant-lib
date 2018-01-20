#ifndef BISECTIONTREEUNION_H
#define BISECTIONTREEUNION_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "bisectiontree.h"
#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _Tp=ibex::IntervalVector>
class BisectionTreeUnion : public BisectionTree<_Tp>
{
public:

    BisectionTreeUnion(std::vector<Maze<_Tp>*> maze_list):BisectionTree<_Tp>(maze_list){}
    BisectionTreeUnion(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B):BisectionTree<_Tp>(maze_A, maze_B){}
    BisectionTreeUnion(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B, Maze<_Tp>* maze_C):BisectionTree<_Tp>(maze_A, maze_B, maze_C){}

    BisectionTreeUnion(std::vector<BisectionTree<_Tp>*> bisectionTree_list):BisectionTree<_Tp>(bisectionTree_list){}
    BisectionTreeUnion(BisectionTree<_Tp>* bisectionTree_A, BisectionTree<_Tp>* bisectionTree_B):BisectionTree<_Tp>(bisectionTree_A, bisectionTree_B){}
    BisectionTreeUnion(BisectionTree<_Tp>* bisectionTree_A, BisectionTree<_Tp>* bisectionTree_B, BisectionTree<_Tp>* bisectionTree_C):BisectionTree<_Tp>(bisectionTree_A, bisectionTree_B, bisectionTree_C){}

    BisectionTreeUnion(std::vector<Maze<_Tp>*> maze_list, std::vector<BisectionTree<_Tp>*> bisectionTree_list):BisectionTree<_Tp>(maze_list, bisectionTree_list){}
    BisectionTreeUnion(Maze<_Tp>* maze_A, BisectionTree<_Tp>* bisectionTree_A):BisectionTree<_Tp>(maze_A, bisectionTree_A){}

    bool eval(Pave<_Tp> *pave);
};

using BisectionTreeUnionPPL = BisectionTreeUnion<Parma_Polyhedra_Library::C_Polyhedron>;
using BisectionTreeUnionIBEX = BisectionTreeUnion<ibex::IntervalVector>;

}

#include "bisectiontreeunion.tpp"

#endif // BISECTIONTREEUNION_H
