#ifndef BISECTIONTREEINTER_H
#define BISECTIONTREEINTER_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "bisectiontree.h"
#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _Tp=ibex::IntervalVector>
class BisectionTreeInter : public BisectionTree<_Tp>
{
public:
    BisectionTreeInter(std::vector<Maze<_Tp>*> maze_list):BisectionTree<_Tp>(maze_list){}
    BisectionTreeInter(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B):BisectionTree<_Tp>(maze_A, maze_B){}
    BisectionTreeInter(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B, Maze<_Tp>* maze_C):BisectionTree<_Tp>(maze_A, maze_B, maze_C){}

    BisectionTreeInter(std::vector<BisectionTree<_Tp>*> bisectionTree_list):BisectionTree<_Tp>(bisectionTree_list){}
    BisectionTreeInter(BisectionTree<_Tp>* bisectionTree_A, BisectionTree<_Tp>* bisectionTree_B):BisectionTree<_Tp>(bisectionTree_A, bisectionTree_B){}
    BisectionTreeInter(BisectionTree<_Tp>* bisectionTree_A, BisectionTree<_Tp>* bisectionTree_B, BisectionTree<_Tp>* bisectionTree_C):BisectionTree<_Tp>(bisectionTree_A, bisectionTree_B, bisectionTree_C){}

    BisectionTreeInter(std::vector<Maze<_Tp>*> maze_list, std::vector<BisectionTree<_Tp>*> bisectionTree_list):BisectionTree<_Tp>(maze_list, bisectionTree_list){}
    BisectionTreeInter(Maze<_Tp>* maze_A, BisectionTree<_Tp>* bisectionTree_A):BisectionTree<_Tp>(maze_A, bisectionTree_A){}

    bool eval(Pave<_Tp> *pave);
};

using BisectionTreeInterPPL = BisectionTreeInter<Parma_Polyhedra_Library::C_Polyhedron>;
using BisectionTreeInterIBEX = BisectionTreeInter<ibex::IntervalVector>;

}

#include "bisectiontreeinter.tpp"

#endif // BISECTIONTREEINTER_H
