#ifndef BISECTIONTREE_H
#define BISECTIONTREE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "maze.h"
#include "pave.h"

namespace invariant {

template <typename _Tp=ibex::IntervalVector>
class BisectionTree
{
public:
    /**
     * @brief BisectionTree constructor
     */
    BisectionTree(std::vector<Maze<_Tp>*> maze_list);
    BisectionTree(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B);
    BisectionTree(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B, Maze<_Tp>* maze_C);

    BisectionTree(std::vector<BisectionTree<_Tp>*> bisectionTree_list);
    BisectionTree(BisectionTree<_Tp>* bisectionTree_A, BisectionTree<_Tp>* bisectionTree_B);
    BisectionTree(BisectionTree<_Tp>* bisectionTree_A, BisectionTree<_Tp>* bisectionTree_B, BisectionTree<_Tp>* bisectionTree_C);

    BisectionTree(std::vector<Maze<_Tp>*> maze_list, std::vector<BisectionTree<_Tp>*> bisectionTree_list);
    BisectionTree(Maze<_Tp>* maze_A, BisectionTree<_Tp>* bisectionTree_A);

    /**
     * @brief Copy constructor
     * @param bisect_tree
     */
    BisectionTree(BisectionTree<_Tp> &bisect_tree);

    /**
     * @brief BisectionTree destructor
     */
//    virtual ~BisectionTree()=0;

    /**
     * @brief add_maze
     * @param maze
     */
    void add_maze(Maze<_Tp> * maze);

    /**
     * @brief add_bisection_tree
     * @param bisection_tree
     */
    void add_bisection_tree(BisectionTree<_Tp> * bisection_tree);

    /**
     * @brief get_children_list
     * @return
     */
    const std::vector<BisectionTree<_Tp>*>& get_children_list() const;

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
    virtual bool eval(Pave<_Tp> *pave)=0;

protected:
    std::vector<BisectionTree<_Tp>*> m_children_list;
    std::vector<Maze<_Tp>*> m_maze_list;

};

using BisectionTreePPL = BisectionTree<Parma_Polyhedra_Library::C_Polyhedron>;
using BisectionTreeIBEX = BisectionTree<ibex::IntervalVector>;

}


#include "bisectiontree.tpp"

#endif // BISECTIONTREE_H

