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

int main(int argc, char *argv[])
{
    ibex::Variable z, dz;

    IntervalVector space(2);
    space[0] = ibex::Interval(-20, 80.0);
    space[1] = ibex::Interval(-0.4,0.4);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    // ****** Dynamics ******* //
    double screw_thread = 1.75e-3;
    double tick_per_turn = 48;
    double piston_diameter = 0.05;
    double tick_to_volume = (screw_thread/tick_per_turn)*pow(piston_diameter/2.0, 2)*M_PI;

    // Physical parameters
    double g = 9.81;
    double rho = 1025.0;
    double m = 9.045*2.;
    double diam = 0.24;
//    double chi = tick_to_volume*30.0;
    double chi = -tick_to_volume*10.0;
    double Cf = M_PI*pow(diam/2.0,2);

    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;

    // Command
    ibex::Interval Vp(-2.148e-5, 1.503e-4);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner_fwd(&subpaving, FULL_WALL);
    dom_inner_fwd.set_border_path_in(true);
    dom_inner_fwd.set_border_path_out(false);

    invariant::Domain<> dom_inner_bwd(&subpaving, FULL_WALL);
    dom_inner_bwd.set_border_path_in(true);
    dom_inner_bwd.set_border_path_out(false);

    // State vector: (z, dz)
    ibex::Function f(z, dz, Return(dz,
                                   (-A*Vp+A*chi*z-B*abs(dz)*dz)));

    ibex::Function f_inner_fwd_1(z, dz, -Return(dz,
                                                -A*(Vp.lb()-chi*z)-B*abs(dz)*dz));
    ibex::Function f_inner_fwd_2(z, dz, -Return(dz,
                                                -A*(Vp.ub()-chi*z)-B*abs(dz)*dz));

    ibex::Function f_inner_bwd_1(z, dz, Return(dz,
                                                -A*(Vp.lb()-chi*z)-B*abs(dz)*dz));
    ibex::Function f_inner_bwd_2(z, dz, Return(dz,
                                                -A*(Vp.ub()-chi*z)-B*abs(dz)*dz));

    std::vector<ibex::Function *> f_inner_fwd{&f_inner_fwd_1, &f_inner_fwd_2};
    std::vector<ibex::Function *> f_inner_bwd{&f_inner_bwd_1, &f_inner_bwd_2};

    DynamicsFunction dyn_outer(&f, FWD_BWD);
    DynamicsFunction dyn_inner_fwd(f_inner_fwd, FWD);
    DynamicsFunction dyn_inner_bwd(f_inner_bwd, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner_fwd(&dom_inner_fwd, &dyn_inner_fwd);
    invariant::Maze<> maze_inner_bwd(&dom_inner_bwd, &dyn_inner_bwd);

    BooleanTreeUnion<> *bisection_inner = new BooleanTreeUnion<>(&maze_inner_fwd, &maze_inner_bwd);
    BooleanTreeInter<> *bisection = new BooleanTreeInter<>(&maze_outer, bisection_inner);
    subpaving.set_bisection_tree(bisection);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    omp_set_num_threads(1);

    for(int i=0; i<16; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
        maze_inner_fwd.contract();
        maze_inner_bwd.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    vector<invariant::MazeIBEX*> list_outer{&maze_outer};
    vector<invariant::MazeIBEX*> list_inner{&maze_inner_fwd, &maze_inner_bwd};
    VibesMaze v_maze("piston_kernel", list_outer, list_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);
    v_maze.show();
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

//    VibesMaze v_maze_inner_fwd("piston_kernel_inner_fwd", &maze_inner_fwd);
//    v_maze_inner_fwd.setProperties(0, 0, 512, 512);
//    v_maze_inner_fwd.show();

//    VibesMaze v_maze_inner_bwd("piston_kernel_inner_bwd", &maze_inner_bwd);
//    v_maze_inner_bwd.setProperties(0, 0, 512, 512);
//    v_maze_inner_bwd.show();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(68.);
//    position_info[1] = ibex::Interval(0.382, 0.392);
//    v_maze.show_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}

