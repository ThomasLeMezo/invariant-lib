#ifndef BOOLEANTREENOT_H
#define BOOLEANTREENOT_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "booleantree.h"
#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _TpR=ibex::IntervalVector,typename _TpF=ibex::IntervalVector,typename _TpD=ibex::IntervalVector>
class BooleanTreeNot : public BooleanTree<_TpR,_TpF,_TpD>
{
public:

    BooleanTreeNot(Maze<_TpR,_TpF,_TpD>* maze):BooleanTree<_TpR,_TpF,_TpD>(maze){}

    BooleanTreeNot(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree):BooleanTree<_TpR,_TpF,_TpD>(bisectionTree){}

    bool eval_bisection(Pave<_TpR,_TpF,_TpD> *pave);

    bool eval_set_empty(Pave<_TpR,_TpF,_TpD> *pave);
};

using BooleanTreeNotPPL = BooleanTreeNot<Parma_Polyhedra_Library::C_Polyhedron>;
using BooleanTreeNotIBEX = BooleanTreeNot<ibex::IntervalVector>;

}

#include "booleantreenot.tpp"



#endif // BOOLEANTREENOT_H
