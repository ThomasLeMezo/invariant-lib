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
    space[0] = ibex::Interval(10, 20); // depth
    space[1] = ibex::Interval(-0.15, 0.15); // velocity

    /// ************************
    ///         Domain
    /// ************************
    invariant::SmartSubPaving<> subpaving(space);

    ibex::IntervalVector target(2);
    target[0] = ibex::Interval(14.8, 15.2);
    target[1] = ibex::Interval(-0.01, 0.01);

    // Bwd

    Function f_id("x[2]", "(x[0], x[1])");
    SepFwdBwd sep_dom(f_id, target);
    SepNot sep_dom_not(sep_dom);

    invariant::Domain<> dom_outer(&subpaving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);
    dom_outer.set_sep_input(&sep_dom_not);

    invariant::Domain<> dom_inner(&subpaving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    dom_inner.set_sep_output(&sep_dom_not);

    // Basin
    invariant::Domain<> dom_outer_basin(&subpaving, FULL_WALL);
    dom_outer_basin.set_border_path_in(false);
    dom_outer_basin.set_border_path_out(false);

    invariant::Domain<> dom_inner_basin(&subpaving, FULL_DOOR);
    dom_inner_basin.set_border_path_in(true);
    dom_inner_basin.set_border_path_out(true);

    /// ************************
    ///         Dynamics
    /// ************************

    // ****** Dynamics ******* //
    double screw_thread = 1.75e-3;
    double tick_per_turn = 48;
    double piston_diameter = 0.05;
    double tick_to_volume = (screw_thread/tick_per_turn)*pow(piston_diameter/2.0, 2)*M_PI;
    double g = 9.81;
    double rho = 1025.0;
    double m = 9.045*2.;
    double diam = 0.24;
    double chi = tick_to_volume*30.0;
    double x2_target = 15.0; // Desired depth
    double beta = 0.04/M_PI_2;
    double alpha = 1.0;
    double gamma = beta/alpha;
    double l = 1.;
    double Cf = M_PI*pow(diam/2.0,2);
    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;
    ibex::Interval Vp(-2.148e-5, 1.503e-4);
    ibex::Function e(x2, (x2_target-x2)/alpha);
    ibex::Function y(x1, x2, (x1-beta*atan(e(x2))));
    ibex::Function D(x2, (1+pow(e(x2), 2)));
    ibex::Function u(x1, x2, min(max((1/A*(l*y(x1,x2)-(B*abs(x1)+gamma/D(x2))*x1)+chi*x2),Vp.lb()),Vp.ub()));
    ibex::Interval z_noise(0);//(-1e-3, 1e-3);
    ibex::Interval dz_noise(0);//(-5e-3, 5e-3);
    ibex::Function f(x2, x1, Return(x1,(-A*(u(x1+dz_noise,x2+z_noise)-chi*x2)-B*abs(x1)*x1)));

    DynamicsFunction dyn(&f, FWD); // Duplicate because of simultaneous access of f (semaphore on DynamicsFunction)
    DynamicsFunction dyn_basin(&f, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

//    invariant::Maze<> maze_outer_basin(&dom_outer_basin, &dyn_basin);
//    invariant::Maze<> maze_inner_basin(&dom_inner_basin, &dyn_basin);

//    invariant::BooleanTreeUnionIBEX outer_graph(&maze_outer, &maze_outer_basin);
//    invariant::BooleanTreeUnionIBEX inner_graph(&maze_inner, &maze_inner_basin);
//    invariant::BooleanTreeInterIBEX bisect_graph(&outer_graph, &inner_graph);
//    subpaving.set_bisection_tree(&bisect_graph);

//    dom_inner_basin.add_maze_initialization_inter(&maze_inner);
//    dom_outer_basin.add_maze_initialization_union(&maze_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);

    for(int i=0; i<15; i++){
        cout << i << endl;
        subpaving.bisect();

        maze_outer.contract();
        maze_inner.contract();

//        maze_outer_basin.contract();
//        maze_inner_basin.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("piston_bwd", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(true);
//    v_maze.set_offset(-15., 0.);
    v_maze.show();
    v_maze.drawBox(target, "r[]");
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

//    VibesMaze v_maze_basin("piston_stay", &maze_outer_basin, &maze_inner_basin);
//    v_maze_basin.setProperties(0, 0, 1000, 800);
//    v_maze_basin.set_enable_cone(false);
////    v_maze_basin.set_offset(-15., 0.);
//    v_maze_basin.show();
//    v_maze_basin.drawBox(target, "black[r]");
//    v_maze_basin.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

    vibes::endDrawing();
}
