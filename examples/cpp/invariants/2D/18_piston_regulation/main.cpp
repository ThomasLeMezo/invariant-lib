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
    space[0] = ibex::Interval(-0.1, 0.1);
    space[1] = ibex::Interval(0., 80);

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
    double x2_target = 15.0; // Desired depth
    double beta = 0.04/M_PI_2;
    double alpha = 1.0;
    double gamma = beta/alpha;

    double r = -1.0;
    double l = 10.;
//    cout << "l1 = " << l1 << endl;
//    cout << "l2 = " << l2 << endl;

    double Cf = M_PI*pow(diam/2.0,2);

    // Command
    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;

    ibex::Interval Vp(-2.148e-5, 1.503e-4);
//    ibex::Interval Vp(-2.148e-10, 1.503e-10);

    ibex::Function e(x2, (x2_target-x2)/alpha);
    ibex::Function y(x1, x2, (x1-beta*atan(e(x2))));
    ibex::Function D(x2, (1+pow(e(x2), 2)));

    ibex::Function u(x1, x2, min(max((1/A*(l*y(x1,x2)-(B*abs(x1)+gamma/D(x2))*x1)+chi*x2),Vp.lb()),Vp.ub()));

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(true);

    // Evolution function
    ibex::Function f(x1, x2, Return((-A*(u(x1,x2)-chi*x2)-B*abs(x1)*x1),
                                    x1));
    ibex::Function f_inner(x1, x2, Return((-A*(u(x1,x2)-chi*x2)-B*abs(x1)*x1),
                                    x1));

    DynamicsFunction dyn_outer(&f, BWD);
    DynamicsFunction dyn_inner(&f_inner, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(int i=0; i<10; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("piston_regulation", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(true);
    ibex::IntervalVector white_space(2);
    white_space[0] = ibex::Interval(-0.01, 0.01);
    white_space[1] = ibex::Interval::ZERO;
    v_maze.drawBox(space, "white[white]");
    v_maze.show();
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");


    vibes::endDrawing();

}
