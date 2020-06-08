#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"
#include "booleantreeinter.h"

#include "ibex_SepFwdBwd.h"
#include "ibex_SepNot.h"

#include <iostream>
#include "language.h"
#include "spacefunction.h"

using namespace std;
using namespace ibex;
using namespace invariant;

void test1(){
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-1, 1);
    space[1] = ibex::Interval(-1.3,1.3);

    /// ************************
    ///         Domain
    /// ************************
    invariant::SmartSubPaving<> subpaving(space);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.2;

    // Outer
    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    Function f_sep_dom_outer(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd sep_dom_output(f_sep_dom_outer, LEQ);
    dom_outer.set_sep(&sep_dom_output);

    // Inner
    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(false);

    Function f_sep_dom_inner(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd sep_dom_input(f_sep_dom_inner, GEQ);
    dom_inner.set_sep_input(&sep_dom_input);

    // Outer Basin
    invariant::Domain<> dom_outer_basin(&subpaving, FULL_WALL);
    dom_outer_basin.set_border_path_in(false);
    dom_outer_basin.set_border_path_out(false);

    invariant::Domain<> dom_inner_basin(&subpaving, FULL_DOOR);
    dom_inner_basin.set_border_path_in(true);
    dom_inner_basin.set_border_path_out(true);

    /// ************************
    ///         Dynamics
    /// ************************
    ibex::Function f(x1, x2,Return(0.5*x1+x1*x1-x2*x2,
                                            -0.5*x2+x1*x1));
    ibex::Function f2(x1, x2,Return(0.5*x1+x1*x1-x2*x2,
                                            -0.5*x2+x1*x1));
    DynamicsFunction dyn(&f, FWD); // Duplicate because of simultaneous access of f (semaphore on DynamicsFunction)
    DynamicsFunction dyn_basin(&f2, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    invariant::Maze<> maze_outer_basin(&dom_outer_basin, &dyn_basin);
    invariant::Maze<> maze_inner_basin(&dom_inner_basin, &dyn_basin);

    invariant::BooleanTreeUnionIBEX outer_graph(&maze_outer, &maze_outer_basin);
    invariant::BooleanTreeUnionIBEX inner_graph(&maze_inner, &maze_inner_basin);
    invariant::BooleanTreeInterIBEX bisect_graph(&outer_graph, &inner_graph);
    subpaving.set_bisection_tree(&bisect_graph);

    dom_inner_basin.add_maze_initialization_inter(&maze_inner);
    dom_outer_basin.add_maze_initialization_union(&maze_outer);

    vibes::beginDrawing();
    VibesMaze v_maze("Invariant", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    VibesMaze v_maze_basin("Basin", &maze_outer_basin, &maze_inner_basin);
    v_maze_basin.setProperties(0, 0, 1000, 800);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);

    for(int i=0; i<20; i++){
        cout << i << endl;
        double time_local = omp_get_wtime();

        subpaving.bisect();
        maze_outer.contract();
        maze_inner.contract();

        maze_outer_basin.contract();
        maze_inner_basin.contract();

        // Stat
        double t = omp_get_wtime() - time_local;
        double v_outer =  v_maze_basin.get_volume();
        double v_inner = v_maze_basin.get_volume(true);
        v_maze_basin.add_stat(i, t, v_outer, v_inner);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "red", "");

    v_maze_basin.show();
    v_maze_basin.drawCircle(x1_c, x2_c, r, "red", "");
    v_maze_basin.save_stat_to_file("stat.txt");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.5);
//    position_info[1] = ibex::Interval(-0.64, -0.59);
//    v_maze.show_room_info(&maze_outer, position_info);
    vibes::endDrawing();
}

void test2(){
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-0.5, 0.5);
    space[1] = ibex::Interval(-0.5,0.5);

    ibex::Function f_positive(x1, x2,Return(0.5*x1+x1*x1-x2*x2,
                                            -0.5*x2+x1*x1));

    ibex::Function f_negative(x1, x2,-Return(0.5*x1+x1*x1-x2*x2,
                                            -0.5*x2+x1*x1));

    largest_positive_invariant(space, &f_negative, 15, "kleene_ext1");
  //  largest_positive_invariant(space, &f_positive, 15, "dipole_positive");

  //  largest_invariant(space, &f_positive, &f_negative, 15, "dipole_invariant");
}

int main(int argc, char *argv[]){
//    test1();
    test2();
}
