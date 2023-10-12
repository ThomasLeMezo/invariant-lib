#ifndef EULERIANMAZE_H
#define EULERIANMAZE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <ibex_Sep.h>

#include <maze.h>
#include <smartSubPaving.h>
#include <dynamicsFunction.h>

#include <iostream>

namespace invariant {

template <typename _TpR, typename _TpF, typename _TpD> class EulerianMaze;
using EulerianMazePPL = EulerianMaze<Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron>;
using EulerianMazeIBEX = EulerianMaze<ibex::IntervalVector,ibex::IntervalVector,ibex::IntervalVector>;
using EulerianMazeEXP = EulerianMaze<ibex::IntervalVector,ExpVF,ExpPoly>;

template <typename _TpR=ibex::IntervalVector, typename _TpF=ibex::IntervalVector, typename _TpD=ibex::IntervalVector>
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
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, bool with_inner=true, bool copy_function=true);
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, ibex::Sep* sepB, bool with_inner=true, bool copy_function=true);
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, ibex::Sep* sepB, ibex::Sep* sepC, bool with_inner=true, bool copy_function=true);
    EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, const std::vector<ibex::Sep*> &separator_list, bool with_inner=true, bool copy_function=true);

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
    Maze<_TpR,_TpF,_TpD> * get_maze_outer(size_t id);

    /**
     * @brief get_maze_inner
     * @param id
     * @return
     */
    Maze<_TpR,_TpF,_TpD> * get_maze_inner(size_t id);

    /**
     * @brief get_maze_inner
     * @return
     */
    std::vector<Maze<_TpR,_TpF,_TpD>*> get_maze_inner();

    /**
     * @brief get_maze_outer
     * @return
     */
    std::vector<Maze<_TpR,_TpF,_TpD>*> get_maze_outer();

    /**
     * @brief get_maze_outer_fwd
     * @return
     */
    std::vector<Maze<_TpR,_TpF,_TpD>*> get_maze_outer_fwd();

    /**
     * @brief get_maze_outer_bwd
     * @return
     */
    std::vector<Maze<_TpR,_TpF,_TpD>*> get_maze_outer_bwd();

    /**
     * @brief get_maze_inner_fwd
     * @return
     */
    std::vector<Maze<_TpR,_TpF,_TpD>*> get_maze_inner_fwd();

    /**
     * @brief get_maze_inner_bwd
     * @return
     */
    std::vector<Maze<_TpR,_TpF,_TpD>*> get_maze_inner_bwd();

    /**
     * @brief get_number_separators
     * @return
     */
    size_t get_number_maze() const;

    /**
     * @brief get_paving
     * @return
     */
    SmartSubPaving<_TpR,_TpF,_TpD> * get_paving();

private:
    std::vector<ibex::Sep*> m_separator_list;

    SmartSubPaving<_TpR,_TpF,_TpD> *m_paving=nullptr;
    std::vector<DynamicsFunction*> m_dyn_list;

    std::vector<Domain<_TpR,_TpF,_TpD>*> m_dom_list;

    std::vector<Maze<_TpR,_TpF,_TpD>*> m_maze_outer_list;
    std::vector<Maze<_TpR,_TpF,_TpD>*> m_maze_inner_list;

    std::vector<Maze<_TpR,_TpF,_TpD>*> m_maze_outer_fwd_list;
    std::vector<Maze<_TpR,_TpF,_TpD>*> m_maze_inner_fwd_list;
    std::vector<Maze<_TpR,_TpF,_TpD>*> m_maze_outer_bwd_list;
    std::vector<Maze<_TpR,_TpF,_TpD>*> m_maze_inner_bwd_list;

    std::vector<Domain<_TpR,_TpF,_TpD>*> m_dom_outer_fwd_list;
    std::vector<Domain<_TpR,_TpF,_TpD>*> m_dom_outer_bwd_list;
    std::vector<Domain<_TpR,_TpF,_TpD>*> m_dom_inner_fwd_list;
    std::vector<Domain<_TpR,_TpF,_TpD>*> m_dom_inner_bwd_list;

    std::vector<ibex::Sep*> m_sep_not_list;

    std::vector<BooleanTree<_TpR,_TpF,_TpD>*> m_boolean_tree_list;

    bool m_copy_function = true;
    bool m_with_inner = true;
};

template<typename _TpR, typename _TpF, typename _TpD>
size_t EulerianMaze<_TpR,_TpF,_TpD>::get_number_maze() const{
    return m_maze_outer_list.size();
}

template<typename _TpR, typename _TpF, typename _TpD>
SmartSubPaving<_TpR,_TpF,_TpD>* EulerianMaze<_TpR,_TpF,_TpD>::get_paving(){
    return m_paving;
}

template<typename _TpR, typename _TpF, typename _TpD>
std::vector<Maze<_TpR,_TpF,_TpD>*> EulerianMaze<_TpR,_TpF,_TpD>::get_maze_outer(){
    return m_maze_outer_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
std::vector<Maze<_TpR,_TpF,_TpD>*> EulerianMaze<_TpR,_TpF,_TpD>::get_maze_outer_fwd(){
    return m_maze_outer_fwd_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
std::vector<Maze<_TpR,_TpF,_TpD>*> EulerianMaze<_TpR,_TpF,_TpD>::get_maze_outer_bwd(){
    return m_maze_outer_bwd_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
std::vector<Maze<_TpR,_TpF,_TpD>*> EulerianMaze<_TpR,_TpF,_TpD>::get_maze_inner_fwd(){
    return m_maze_inner_fwd_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
std::vector<Maze<_TpR,_TpF,_TpD>*> EulerianMaze<_TpR,_TpF,_TpD>::get_maze_inner_bwd(){
    return m_maze_inner_bwd_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
std::vector<Maze<_TpR,_TpF,_TpD>*> EulerianMaze<_TpR,_TpF,_TpD>::get_maze_inner(){
    return m_maze_inner_list;
}

}


#include "eulerianmaze.tpp"

#endif // EULERIANMAZE_H
