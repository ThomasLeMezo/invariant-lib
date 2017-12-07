#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include <cstring>
#include <omp.h>

#include "vtkmazeppl.h"

#include "ppl.hh"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x, y, theta;

    IntervalVector space(3);
    space[0] = ibex::Interval(-10, 10);
    space[1] = ibex::Interval(-10, 10);
    space[2] = ibex::Interval(0, 10);

    ibex::Interval v = ibex::Interval(0.1);
    ibex::Interval u = ibex::Interval(0.3);

    SmartSubPavingPPL paving(space);

    // ****** Domain Outer ******* //
    DomainPPL dom_outer(&paving, FULL_WALL);

    double x_c, y_c, theta_c, r;
    x_c = 0.0;
    y_c = 0.0;
    theta_c = 0.5;
    r = 1;

    Function f_sep_outer(x, y, theta, pow(x-x_c, 2)+pow(y-y_c, 2) + pow(theta-theta_c, 2) - pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x, y, theta, Return(v*cos(theta),
                                            v*sin(theta),
                                            u));
    Dynamics_Function dyn_outer(&f, FWD);

    // ******* Mazes ********* //
    MazePPL maze_outer(&dom_outer, &dyn_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    omp_set_num_threads(1);
    for(int i=0; i<11; i++){
        paving.bisect();
        cout << i << " outer - ";
        maze_outer.contract();
        cout << " - paving = " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VtkMazePPL vtkMazePPL("DubinsPPL");
    vtkMazePPL.show_maze(&maze_outer);
}
