#include "eulerianmaze.h"
#include "ibex_SepNot.h"

#include "booleantreeinter.h"
#include "booleantreeunion.h"

namespace invariant {

template <typename _Tp>
EulerianMaze<_Tp>::EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, bool with_inner, bool copy_function){
    m_separator_list.push_back(sepA);
    m_copy_function = copy_function;
    m_with_inner = with_inner;
    init_mazes(space, f_dyn);
}

template <typename _Tp>
EulerianMaze<_Tp>::EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, ibex::Sep* sepB, bool with_inner, bool copy_function){
    m_separator_list.push_back(sepA);
    m_separator_list.push_back(sepB);
    m_copy_function = copy_function;
    m_with_inner = with_inner;
    init_mazes(space, f_dyn);
}

template <typename _Tp>
EulerianMaze<_Tp>::EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, ibex::Sep* sepA, ibex::Sep* sepB, ibex::Sep* sepC, bool with_inner, bool copy_function){
    m_separator_list.push_back(sepA);
    m_separator_list.push_back(sepB);
    m_separator_list.push_back(sepC);
    m_copy_function = copy_function;
    m_with_inner = with_inner;
    init_mazes(space, f_dyn);
}

template <typename _Tp>
EulerianMaze<_Tp>::EulerianMaze(const ibex::IntervalVector &space, ibex::Function *f_dyn, const std::vector<ibex::Sep*> &separator_list, bool with_inner, bool copy_function){
    m_separator_list.insert(m_separator_list.end(), separator_list.begin(), separator_list.end());
    m_copy_function = copy_function;
    m_with_inner = with_inner;
    init_mazes(space, f_dyn);
}

template <typename _Tp>
EulerianMaze<_Tp>::~EulerianMaze(){
    if(m_paving!=nullptr){
        delete(m_paving);
        m_paving=nullptr;
    }

    for(Maze<_Tp>* maze:m_maze_outer_list)
        delete(maze);
    for(Maze<_Tp>* maze:m_maze_inner_list)
        delete(maze);

    for(DynamicsFunction* dyn:m_dyn_list)
        delete(dyn);

    for(Domain<_Tp>* dom:m_dom_list)
        delete(dom);

    for(ibex::Sep* sep:m_sep_not_list)
        delete(sep);

    for(BooleanTree<_Tp>* tree:m_boolean_tree_list)
        delete(tree);
}

template <typename _Tp>
void EulerianMaze<_Tp>::init_mazes(const ibex::IntervalVector &space, ibex::Function *f_dyn){
    // SmartSubPaving
    m_paving = new SmartSubPaving<_Tp>(space);

    // Dynamics
    // ToDo when thread safe function

    // Domains
    size_t nb_sep = m_separator_list.size();

    /// ************** FWD ************** ///
    for(size_t i=0; i<(size_t)std::max((int)nb_sep-1, 1); i++){
        // Outer
        invariant::Domain<_Tp> *dom_outer = new invariant::Domain<_Tp>(m_paving, FULL_WALL);
        dom_outer->set_border_path_in(false);
        dom_outer->set_border_path_out(false);
        dom_outer->set_sep_input(m_separator_list[i]);
        m_dom_list.push_back(dom_outer);
        m_dom_outer_fwd_list.push_back(dom_outer);
        ibex::Function *f1;
        if(m_copy_function)
            f1 = new ibex::Function(*f_dyn);
        else
            f1=f_dyn;
        DynamicsFunction *dyn1 = new DynamicsFunction(f1, FWD);
        m_dyn_list.push_back(dyn1);
        invariant::Maze<_Tp> *maze_outer_fwd = new invariant::Maze<_Tp>(dom_outer, dyn1);
        m_maze_outer_list.push_back(maze_outer_fwd);
        m_maze_outer_fwd_list.push_back(maze_outer_fwd);

        // Inner
        if(m_with_inner){
            invariant::Domain<_Tp> *dom_inner = new invariant::Domain<_Tp>(m_paving, FULL_DOOR);
            dom_inner->set_border_path_in(true);
            dom_inner->set_border_path_out(true);
            ibex::SepNot *sepNot = new ibex::SepNot(*m_separator_list[i]);
            m_sep_not_list.push_back(sepNot);
            dom_inner->set_sep_input(sepNot);
            m_dom_list.push_back(dom_inner);
            m_dom_inner_fwd_list.push_back(dom_inner);
            ibex::Function *f2;
            if(m_copy_function)
                f2 = new ibex::Function(*f_dyn);
            else
                f2=f_dyn;
            DynamicsFunction *dyn2 = new DynamicsFunction(f2, FWD);
            m_dyn_list.push_back(dyn2);
            invariant::Maze<_Tp> *maze_inner_fwd = new invariant::Maze<_Tp>(dom_inner, dyn2);
            m_maze_inner_list.push_back(maze_inner_fwd);
            m_maze_inner_fwd_list.push_back(maze_inner_fwd);
        }
    }

    /// ************** BWD ************** ///
    for(size_t i=nb_sep-1; i>0; i--){
        // Outer
        invariant::Domain<_Tp> *dom_outer = new invariant::Domain<_Tp>(m_paving, FULL_WALL);
        dom_outer->set_border_path_in(false);
        dom_outer->set_border_path_out(false);
        dom_outer->set_sep_output(m_separator_list[i]);
        m_dom_list.push_back(dom_outer);
        m_dom_outer_bwd_list.push_back(dom_outer);
        ibex::Function *f1;
        if(m_copy_function)
            f1 = new ibex::Function(*f_dyn);
        else
            f1=f_dyn;
        DynamicsFunction *dyn1 = new DynamicsFunction(f1, BWD);
        m_dyn_list.push_back(dyn1);
        invariant::Maze<_Tp> *maze_outer_bwd = new invariant::Maze<_Tp>(dom_outer, dyn1);
        m_maze_outer_list.push_back(maze_outer_bwd);
        m_maze_outer_bwd_list.push_back(maze_outer_bwd);

        // Inner
        if(m_with_inner){
            invariant::Domain<_Tp> *dom_inner = new invariant::Domain<_Tp>(m_paving, FULL_DOOR);
            dom_inner->set_border_path_in(true);
            dom_inner->set_border_path_out(true);
            ibex::SepNot *sepNot = new ibex::SepNot(*(m_separator_list[i]));
            m_sep_not_list.push_back(sepNot);
            dom_inner->set_sep_output(sepNot);
            m_dom_list.push_back(dom_inner);
            m_dom_inner_bwd_list.push_back(dom_inner);
            ibex::Function *f2;
            if(m_copy_function)
                f2 = new ibex::Function(*f_dyn);
            else
                f2=f_dyn;
            DynamicsFunction *dyn2 = new DynamicsFunction(f2, BWD);
            m_dyn_list.push_back(dyn2);
            invariant::Maze<_Tp> *maze_inner_bwd = new invariant::Maze<_Tp>(dom_inner, dyn2);
            m_maze_inner_list.push_back(maze_inner_bwd);
            m_maze_inner_bwd_list.push_back(maze_inner_bwd);
        }
    }

    /// ****** Outer Domain ******
    for(size_t i=1; i<m_dom_outer_fwd_list.size(); i++){
        m_dom_outer_fwd_list[i]->add_maze_inter_initial_condition(m_maze_outer_fwd_list[i-1]);
    }
    for(size_t i=1; i<m_dom_outer_bwd_list.size(); i++){
        m_dom_outer_bwd_list[i]->add_maze_inter_initial_condition(m_maze_outer_bwd_list[i-1]);
    }
    if(nb_sep>1){
        m_dom_outer_bwd_list[0]->add_maze_inter_initial_condition(m_maze_outer_fwd_list[m_maze_outer_fwd_list.size()-1]);
        m_dom_outer_fwd_list[0]->add_maze_inter_initial_condition(m_maze_outer_bwd_list[m_maze_outer_bwd_list.size()-1]);
    }

    for(size_t i=0; i<m_dom_outer_fwd_list.size(); i++){
        m_dom_outer_fwd_list[i]->add_maze_inter_father_hull(m_maze_outer_bwd_list[m_maze_outer_bwd_list.size()-1-i]);
        m_dom_outer_bwd_list[i]->add_maze_inter_father_hull(m_maze_outer_fwd_list[m_maze_outer_fwd_list.size()-1-i]);
    }

    /// ****** Inner Domain ******
    if(m_with_inner){
        // Test ("inner" between 2 consecutive sep)
//        for(size_t i=0; i<m_dom_inner_fwd_list.size(); i++){
//            m_dom_inner_fwd_list[i]->add_maze_union(m_maze_inner_bwd_list[m_maze_inner_bwd_list.size()-1-i]);
//            m_dom_inner_bwd_list[i]->add_maze_union(m_maze_inner_fwd_list[m_maze_inner_fwd_list.size()-1-i]);
//        }

        for(size_t i=0; i<nb_sep-1; i++){ // ToDo: improve ?
            m_dom_inner_fwd_list[i]->add_maze_union(m_maze_inner_bwd_list);
            m_dom_inner_bwd_list[i]->add_maze_union(m_maze_inner_fwd_list);

            for(size_t k=0; k<nb_sep-1; k++){
                if(i!=k){
                    m_dom_inner_fwd_list[i]->add_maze_union(m_maze_inner_fwd_list[k]);
                    m_dom_inner_bwd_list[i]->add_maze_union(m_maze_inner_bwd_list[k]);
                }
            }
        }
    }

    std::vector<BooleanTree<_Tp> *> bisection_outer_binome;
    size_t nb_maze_outer_fwd = m_dom_outer_fwd_list.size();
    for(size_t i=0; i<nb_maze_outer_fwd; i++){
        BooleanTreeInter<_Tp> *bisection_outer = new BooleanTreeInter<_Tp>(m_maze_outer_fwd_list[i], m_maze_outer_bwd_list[nb_maze_outer_fwd-i-1]); // Intersection of binome intersection
        bisection_outer_binome.push_back(bisection_outer);
    }

    BooleanTreeUnion<_Tp> *bisection_outer = new BooleanTreeUnion<_Tp>(bisection_outer_binome);
    m_boolean_tree_list.push_back(bisection_outer);

    if(m_with_inner){
        BooleanTreeUnion<_Tp> *bisection_inner = new BooleanTreeUnion<_Tp>(m_maze_inner_list);
        BooleanTreeInter<_Tp> *bisection_total = new BooleanTreeInter<_Tp>(bisection_outer, bisection_inner);
        m_paving->set_bisection_tree(bisection_total);
        m_boolean_tree_list.push_back(bisection_inner);
        m_boolean_tree_list.push_back(bisection_total);
    }
    else
        m_paving->set_bisection_tree(bisection_outer);

    m_boolean_tree_list.insert(m_boolean_tree_list.begin(), bisection_outer_binome.begin(), bisection_outer_binome.end());

    if(m_with_inner){
        BooleanTreeInter<_Tp> *tree_removing_inner = new BooleanTreeInter<_Tp>(m_maze_inner_list);
        for(Maze<_Tp> *maze_inner:m_maze_inner_list)
            maze_inner->set_boolean_tree_removing(tree_removing_inner);
        m_boolean_tree_list.push_back(tree_removing_inner);
    }

    BooleanTreeUnion<_Tp> *tree_removing_outer = new BooleanTreeUnion<_Tp>(m_maze_outer_list);
    for(Maze<_Tp> * maze_outer:m_maze_outer_list)
        maze_outer->set_boolean_tree_removing(tree_removing_outer);
    m_boolean_tree_list.push_back(tree_removing_outer);
}

template <typename _Tp>
void EulerianMaze<_Tp>::bisect(){
    if(m_paving!=nullptr)
        m_paving->bisect();
}

template <typename _Tp>
void EulerianMaze<_Tp>::contract(size_t nb_step){
    std::cout << " Outer Maze" << std::endl;
    for(size_t k=0; k<nb_step; k++){
        for(Maze<_Tp> *maze_outer:m_maze_outer_list)
            maze_outer->contract();
    }
    m_maze_outer_list[0]->contract(); // Loop constraint

    if(m_with_inner){
        std::cout << " Inner Maze" << std::endl;
        for(size_t k=0; k<nb_step; k++){
            for(Maze<_Tp> *maze_inner:m_maze_inner_list)
                maze_inner->contract();
        }
        m_maze_inner_list[0]->contract(); // Loop constraint (?)
    }
}

template <typename _Tp>
Maze<_Tp>* EulerianMaze<_Tp>::get_maze_outer(size_t id){
    return m_maze_outer_list[id];
}

template <typename _Tp>
Maze<_Tp>* EulerianMaze<_Tp>::get_maze_inner(size_t id){
    return m_maze_inner_list[id];
}

}
