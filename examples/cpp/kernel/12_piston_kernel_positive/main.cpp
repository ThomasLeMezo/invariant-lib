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
    // Physical parameters
    double g = 9.81;
    double rho = 1025.0;
    double m = 9.045*2.;
    double diam = 0.24;

    double chi = 2.15e-6;
    double Cf = M_PI*pow(diam/2.0,2);

    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;

    // Command
    ibex::Interval Vp(-2.148e-5, 1.503e-4);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    // State vector: (z, dz)
    ibex::Function f(z, dz, Return(dz,
                                   (-A*Vp+A*chi*z-B*abs(dz)*dz)));

    ibex::Function f_inner1(z, dz, -Return(dz,
                                                -A*(Vp.lb()-chi*z)-B*abs(dz)*dz));
    ibex::Function f_inner2(z, dz, -Return(dz,
                                                -A*(Vp.ub()-chi*z)-B*abs(dz)*dz));

    DynamicsFunction dyn_outer(&f, BWD);
    DynamicsFunction dyn_inner(&f_inner1, &f_inner2, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    omp_set_num_threads(1);

    for(int i=0; i<15; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("piston_kernel_positive", &maze_outer, &maze_inner);
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

#ifdef WITH_IPEGENERATOR
    v_maze.draw_axis("z", "\\dot{z}");
    v_maze.saveIpe("/home/lemezoth/Desktop/");
#endif

    vibes::endDrawing();

}

