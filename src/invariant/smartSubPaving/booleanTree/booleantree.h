#ifndef BISECTIONTREE_H
#define BISECTIONTREE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _TpR, typename _TpF, typename _TpD> class Maze;
template <typename _TpR, typename _TpF, typename _TpD> class Pave;

template <typename _TpR=ibex::IntervalVector, typename _TpF=ibex::IntervalVector, typename _TpD=ibex::IntervalVector>
class BooleanTree
{
public:
    /**
     * @brief BooleanTree constructor
     */
    BooleanTree(const std::vector<Maze<_TpR,_TpF,_TpD>*> &maze_list);
    BooleanTree(Maze<_TpR,_TpF,_TpD>* maze_A);
    BooleanTree(Maze<_TpR,_TpF,_TpD>* maze_A, Maze<_TpR,_TpF,_TpD>* maze_B);
    BooleanTree(Maze<_TpR,_TpF,_TpD>* maze_A, Maze<_TpR,_TpF,_TpD>* maze_B, Maze<_TpR,_TpF,_TpD>* maze_C);

    BooleanTree(const std::vector<BooleanTree<_TpR,_TpF,_TpD>*> &bisectionTree_list);
    BooleanTree(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree);
    BooleanTree(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_B);
    BooleanTree(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_B, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_C);

    BooleanTree(const std::vector<Maze<_TpR,_TpF,_TpD>*> &maze_list, const std::vector<BooleanTree<_TpR,_TpF,_TpD>*> &bisectionTree_list);
    BooleanTree(Maze<_TpR,_TpF,_TpD>* maze_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A);

    /**
     * @brief Copy constructor
     * @param bisect_tree
     */
    BooleanTree(BooleanTree<_TpR,_TpF,_TpD> &bisect_tree);

    /**
     * @brief BooleanTree destructor
     */
//    virtual ~BooleanTree()=0;

    /**
     * @brief add_maze
     * @param maze
     */
    void add_maze(Maze<_TpR,_TpF,_TpD> * maze);

    /**
     * @brief add_bisection_tree
     * @param bisection_tree
     */
    void add_bisection_tree(BooleanTree<_TpR,_TpF,_TpD> * bisection_tree);

    /**
     * @brief get_children_list
     * @return
     */
    const std::vector<BooleanTree<_TpR,_TpF,_TpD>*>& get_children_list() const;

    /**
     * @brief get_maze_list
     * @return
     */
    const std::vector<Maze<_TpR,_TpF,_TpD>*>& get_maze_list() const;

    /**
     * @brief eval if there is a need to bisect
     * @param pave
     * @return
     */
    virtual bool eval_bisection(Pave<_TpR,_TpF,_TpD> *pave)=0;

    /**
     * @brief eval if a pave can be set to empty
     * @param pave
     * @return
     */
    virtual bool eval_set_empty(Pave<_TpR,_TpF,_TpD> *pave)=0;

protected:
    std::vector<BooleanTree<_TpR,_TpF,_TpD>*> m_children_list;
    std::vector<Maze<_TpR,_TpF,_TpD>*> m_maze_list;

};

using BooleanTreePPL = BooleanTree<Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron>;
using BooleanTreeIBEX = BooleanTree<ibex::IntervalVector,ibex::IntervalVector,ibex::IntervalVector>;

}


#include "booleantree.tpp"

#endif // BISECTIONTREE_H

