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

    // ****** Dynamics ******* //
    double screw_thread = 1.75e-3;
    double tick_per_turn = 48;
    double piston_diameter = 0.05;
    double tick_to_volume = (screw_thread/tick_per_turn)*pow(piston_diameter/2.0, 2)*M_PI;

    // Physical parameters
    ibex::Interval g = 9.81;
    ibex::Interval rho = 1025.0 + ibex::Interval(-5,5);
    ibex::Interval m = 9.045*2. + ibex::Interval(-0.1, 0.1);
    double diam = 0.24;

//    double chi = tick_to_volume*30.0;
    ibex::Interval chi = 2.15e-6 + ibex::Interval(-0.1e-6, 0.1e-6);
    cout << "chi = " << tick_to_volume*30.0 << " | " << chi << endl;
//    double chi = -tick_to_volume*10.0;
    ibex::Interval Cf(M_PI*pow(diam/2.0,2));

    ibex::Interval A = g*rho/m;
    ibex::Interval B = 0.5*rho*Cf/m;

    // Command
    ibex::Interval Vp = ibex::Interval(-2.148e-5, 1.503e-4) + ibex::Interval(-0.1e-5, 0.1e-6);

    // No Go zone
    double constraint = 10; // From 0 to 15
    ibex::IntervalVector no_go_zone(2);
    no_go_zone[0] = ibex::Interval(70.0-constraint, 75.0);
    no_go_zone[1] = ibex::Interval(-0.05, 0.05);
    Function f_no_go_zone("x[2]", "(x[0], x[1])");
    SepFwdBwd sep_dom_inner(f_no_go_zone, no_go_zone);
    SepNot sep_dom_outer(sep_dom_inner);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);
    dom_outer.set_sep(&sep_dom_outer);

    invariant::Domain<> dom_inner_fwd(&subpaving, FULL_WALL);
    dom_inner_fwd.set_border_path_in(true);
    dom_inner_fwd.set_border_path_out(false);
    dom_inner_fwd.set_sep(&sep_dom_inner);

    invariant::Domain<> dom_inner_bwd(&subpaving, FULL_WALL);
    dom_inner_bwd.set_border_path_in(true);
    dom_inner_bwd.set_border_path_out(false);
    dom_inner_bwd.set_sep(&sep_dom_inner);

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

    for(int i=0; i<15; i++){
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
    VibesMaze v_maze("piston_kernel_uncertain", list_outer, list_inner);
    v_maze.set_enable_cone(false);

#ifdef WITH_IPEGENERATOR
    v_maze.set_ipe_ratio(112,63);
    v_maze.set_axis_limits(-20., 10., -0.4, 0.2);
    v_maze.set_enable_white_boundary(false);
    v_maze.set_thickness_pen_factor(1e-4);
    v_maze.set_thickness_axis(1e-3);
    v_maze.set_number_digits_x(1);
    v_maze.set_number_digits_y(1);
#endif

    v_maze.show();
    v_maze.drawBox(no_go_zone, "black", "red");

#ifdef WITH_IPEGENERATOR
    v_maze.draw_axis("z", "\\dot{z}");
    v_maze.saveIpe("/home/lemezoth/Desktop/");
#endif

    vibes::endDrawing();

}

