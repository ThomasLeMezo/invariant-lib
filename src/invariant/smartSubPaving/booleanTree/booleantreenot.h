#ifndef BOOLEANTREENOT_H
#define BOOLEANTREENOT_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "booleantree.h"
#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _Tp=ibex::IntervalVector>
class BooleanTreeNot : public BooleanTree<_Tp>
{
public:

    BooleanTreeNot(Maze<_Tp>* maze):BooleanTree<_Tp>(maze){}

    BooleanTreeNot(BooleanTree<_Tp>* bisectionTree):BooleanTree<_Tp>(bisectionTree){}

    bool eval_bisection(Pave<_Tp> *pave);

    bool eval_set_empty(Pave<_Tp> *pave);
};

using BooleanTreeNotPPL = BooleanTreeNot<Parma_Polyhedra_Library::C_Polyhedron>;
using BooleanTreeNotIBEX = BooleanTreeNot<ibex::IntervalVector>;

}

#include "booleantreenot.tpp"



#endif // BOOLEANTREENOT_H
