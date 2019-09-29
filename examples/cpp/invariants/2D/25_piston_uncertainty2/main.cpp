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
#include "language.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(14.7, 15.3); // depth
    space[1] = ibex::Interval(-1.5e-2, 1.5e-2); // velocity

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

    double l = 10.;
    double Cf = M_PI*pow(diam/2.0,2);

    // Command
    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;

    ibex::Interval Vp(-2.148e-5, 1.503e-4);
    ibex::Function e(x2, (x2_target-x2)/alpha);
    ibex::Function y(x1, x2, (x1-beta*atan(e(x2))));
    ibex::Function D(x2, (1+pow(e(x2), 2)));
    ibex::Function u(x1, x2, min(max((1/A*(l*y(x1,x2)-(B*abs(x1)+gamma/D(x2))*x1)+chi*x2),Vp.lb()),Vp.ub()));

    // Evolution function
    ibex::Interval z_noise(-1e-3, 1e-3);
    ibex::Interval dz_noise(-5e-3, 5e-3);
    ibex::Function f(x2, x1, Return(x1,(-A*(u(x1+dz_noise,x2+z_noise)-chi*x2)-B*abs(x1)*x1)));

    largest_positive_invariant(space, &f, 15, "piston_uncertainty_invariant");

}

