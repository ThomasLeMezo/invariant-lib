#ifndef EULERIANMAZE_H
#define EULERIANMAZE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <ibex_Sep.h>

#include <maze.h>
#include <smartSubPaving.h>
#include <dynamics_function.h>

#include <iostream>

namespace invariant {

template <typename _Tp> class EulerianMaze;
using EulerianMazePPL = EulerianMaze<Parma_Polyhedra_Library::C_Polyhedron>;
using EulerianMazeIBEX = EulerianMaze<ibex::IntervalVector>;

template <typename _Tp=ibex::IntervalVector>
class EulerianMaze
{
private:
    /**
     * @brief init_mazes
     */
    void init_mazes(const ibex::IntervalVector &space, ibex::Function *f_dyn);

public:
    /**
     * @brief EulerianMaze constructor
     * @param space
     * @param f_dyn
     * @param sepA
     */
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA);
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, ibex::Sep* sepB);
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, ibex::Sep* sepB, ibex::Sep* sepC);
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, const std::vector<ibex::Sep*> &separator_list);

    ~EulerianMaze();

    /**
     * @brief bisect
     */
    void bisect();

    /**
     * @brief contract
     */
    void contract(size_t nb_step=1);

    /**
     * @brief get_maze_outer
     * @param id
     * @return
     */
    Maze<_Tp> * get_maze_outer(size_t id);

    /**
     * @brief get_maze_inner
     * @param id
     * @return
     */
    Maze<_Tp> * get_maze_inner(size_t id);

private:
    std::vector<ibex::Sep*> m_separator_list;

    SmartSubPaving<_Tp> *m_paving=nullptr;
    std::array<Dynamics_Function*, 2> m_dyn_array;

    std::vector<Domain<_Tp>*> m_dom_list;

    std::vector<Maze<_Tp>*> m_maze_outer_list;
    std::vector<Maze<_Tp>*> m_maze_inner_list;

    std::vector<Maze<_Tp>*> m_maze_outer_fwd_list;
    std::vector<Maze<_Tp>*> m_maze_inner_fwd_list;
    std::vector<Maze<_Tp>*> m_maze_outer_bwd_list;
    std::vector<Maze<_Tp>*> m_maze_inner_bwd_list;

    std::vector<Domain<_Tp>*> m_dom_outer_fwd_list;
    std::vector<Domain<_Tp>*> m_dom_outer_bwd_list;
    std::vector<Domain<_Tp>*> m_dom_inner_fwd_list;
    std::vector<Domain<_Tp>*> m_dom_inner_bwd_list;

    std::vector<ibex::Sep*> m_sep_not_list;

    std::vector<BooleanTree<_Tp>*> m_boolean_tree_list;
};

}


#include "eulerianmaze.tpp"

#endif // EULERIANMAZE_H
