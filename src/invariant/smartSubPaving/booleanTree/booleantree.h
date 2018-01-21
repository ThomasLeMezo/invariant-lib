#ifndef BISECTIONTREE_H
#define BISECTIONTREE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _Tp> class Maze;
template <typename _Tp> class Pave;

template <typename _Tp=ibex::IntervalVector>
class BooleanTree
{
public:
    /**
     * @brief BooleanTree constructor
     */
    BooleanTree(std::vector<Maze<_Tp>*> maze_list);
    BooleanTree(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B);
    BooleanTree(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B, Maze<_Tp>* maze_C);

    BooleanTree(std::vector<BooleanTree<_Tp>*> bisectionTree_list);
    BooleanTree(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B);
    BooleanTree(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B, BooleanTree<_Tp>* bisectionTree_C);

    BooleanTree(std::vector<Maze<_Tp>*> maze_list, std::vector<BooleanTree<_Tp>*> bisectionTree_list);
    BooleanTree(Maze<_Tp>* maze_A, BooleanTree<_Tp>* bisectionTree_A);

    /**
     * @brief Copy constructor
     * @param bisect_tree
     */
    BooleanTree(BooleanTree<_Tp> &bisect_tree);

    /**
     * @brief BooleanTree destructor
     */
//    virtual ~BooleanTree()=0;

    /**
     * @brief add_maze
     * @param maze
     */
    void add_maze(Maze<_Tp> * maze);

    /**
     * @brief add_bisection_tree
     * @param bisection_tree
     */
    void add_bisection_tree(BooleanTree<_Tp> * bisection_tree);

    /**
     * @brief get_children_list
     * @return
     */
    const std::vector<BooleanTree<_Tp>*>& get_children_list() const;

    /**
     * @brief get_maze_list
     * @return
     */
    const std::vector<Maze<_Tp>*>& get_maze_list() const;

    /**
     * @brief eval if there is a need to bisect
     * @param pave
     * @return
     */
    virtual bool eval_bisection(Pave<_Tp> *pave)=0;

    /**
     * @brief eval if a pave can be set to empty
     * @param pave
     * @return
     */
    virtual bool eval_set_empty(Pave<_Tp> *pave)=0;

protected:
    std::vector<BooleanTree<_Tp>*> m_children_list;
    std::vector<Maze<_Tp>*> m_maze_list;

};

using BooleanTreePPL = BooleanTree<Parma_Polyhedra_Library::C_Polyhedron>;
using BooleanTreeIBEX = BooleanTree<ibex::IntervalVector>;

}


#include "booleantree.tpp"

#endif // BISECTIONTREE_H

