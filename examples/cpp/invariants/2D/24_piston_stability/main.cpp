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
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-5, 55); // depth
    space[1] = ibex::Interval(-0.3, 0.3); // velocity

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
    double chi = tick_to_volume*30.0;

    // Regulation parameters
//    double x2_target = 15.0; // Desired depth
    ibex::Interval x2_target = ibex::Interval(0, 50);

    double beta = 0.04/M_PI_2;
    double alpha = 1.0;
    double gamma = beta/alpha;

    double l = 1.;

    double Cf = M_PI*pow(diam/2.0,2);

    // Command
    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;

    ibex::Interval Vp(-2.148e-5, 1.503e-4);

    ibex::Function e(x2, (x2_target-x2)/alpha);
    ibex::Function y(x1, x2, (x1-beta*atan(e(x2))));
    ibex::Function D(x2, (1+pow(e(x2), 2)));

    ibex::Function e1(x2, (x2_target.lb()-x2)/alpha);
    ibex::Function y1(x1, x2, (x1-beta*atan(e1(x2))));
    ibex::Function D1(x2, (1+pow(e1(x2), 2)));

    ibex::Function e2(x2, (x2_target.ub()-x2)/alpha);
    ibex::Function y2(x1, x2, (x1-beta*atan(e2(x2))));
    ibex::Function D2(x2, (1+pow(e2(x2), 2)));

    ibex::Function u(x1, x2, min(max((1/A*(l*y(x1,x2)-(B*abs(x1)+gamma/D(x2))*x1)+chi*x2),Vp.lb()),Vp.ub()));
    ibex::Function u1(x1, x2, min(max((1/A*(l*y1(x1,x2)-(B*abs(x1)+gamma/D1(x2))*x1)+chi*x2),Vp.lb()),Vp.ub()));
    ibex::Function u2(x1, x2, min(max((1/A*(l*y2(x1,x2)-(B*abs(x1)+gamma/D2(x2))*x1)+chi*x2),Vp.lb()),Vp.ub()));

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner_fwd(&subpaving, FULL_WALL);
    dom_inner_fwd.set_border_path_in(true);
    dom_inner_fwd.set_border_path_out(true);

    // Evolution function
    ibex::Function f(x2, x1, -Return(x1,(-A*(u(x1,x2)-chi*x2)-B*abs(x1)*x1)));
    ibex::Function f_inner1_fwd(x2, x1, Return(x1,(-A*(u1(x1,x2)-chi*x2)-B*abs(x1)*x1)));
    ibex::Function f_inner2_fwd(x2, x1, Return(x1,(-A*(u2(x1,x2)-chi*x2)-B*abs(x1)*x1)));

    std::vector<ibex::Function *> f_inner_fwd{&f_inner1_fwd, &f_inner2_fwd};

    DynamicsFunction dyn_outer(&f, BWD);
    DynamicsFunction dyn_inner_fwd(f_inner_fwd, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner_fwd(&dom_inner_fwd, &dyn_inner_fwd);

    // ******* Algorithm ********* //
    omp_set_num_threads(1);
    double time_start = omp_get_wtime();

    for(int i=0; i<16; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
        maze_inner_fwd.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    VibesMaze v_maze("piston_stability", &maze_outer, &maze_inner_fwd);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
    v_maze.set_scale(1., 1.);
    v_maze.show();
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");
    // ToDo: issue with dimension of figure

//    ibex::IntervalVector test(2);
//    test[0] = ibex::Interval(14.72);
//    test[1] = ibex::Interval(8.4e-3);
//    v_maze.set_enable_cone(true);
//    v_maze.show_room_info(&maze_inner_fwd, test);

    vibes::endDrawing();

}

