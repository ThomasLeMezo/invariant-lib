#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[]){
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    /// ************************
    ///         Domain
    /// ************************
    invariant::SmartSubPaving<> subpaving(space);

    double x1_c, x2_c, r;
    x1_c = 0.5;
    x2_c = 0.5;
    r = 1.5;

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
    ibex::Function f(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
    ibex::Function f2(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
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

//    dom_inner_basin.add_maze_inter_initials_condition(&maze_inner);
    dom_outer_basin.add_maze_union(&maze_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    omp_set_num_threads(1);

    for(int i=0; i<14; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
        maze_inner.contract();

        maze_outer_basin.contract();
//        maze_inner_basin.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "r[]");

    VibesMaze v_maze_basin("Basin", &maze_outer_basin/*, &maze_inner_basin*/);
    v_maze_basin.setProperties(0, 0, 1024, 1024);
    v_maze_basin.show();
    v_maze_basin.drawCircle(x1_c, x2_c, r, "r[]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.5);
//    position_info[1] = ibex::Interval(-0.64, -0.59);
//    v_maze.show_room_info(&maze_outer, position_info);
    vibes::endDrawing();



}
