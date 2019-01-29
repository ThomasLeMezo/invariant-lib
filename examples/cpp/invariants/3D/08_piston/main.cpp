#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>
#include "vtkmazeppl.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2, x3;

    IntervalVector space(3);
    space[0] = ibex::Interval(-1, 1); // Velocity
    space[1] = ibex::Interval(0, 30); // Position
    space[2] = -1.7e-4/2.0*ibex::Interval(-1, 1); // Piston volume (m3)

    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);
    invariant::DomainPPL dom(&paving, FULL_DOOR);

//    Function f_sep(x, y, z, pow(x, 2)+pow(y, 2)+pow(z, 2)-pow(0.5, 2));
//    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
//    dom.set_sep(&s);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //

    // Physical parameters
    ibex::Interval g(9.81);
    ibex::Interval rho(1025.0);
    ibex::Interval m(8.8);
    ibex::Interval diam(0.24);
    ibex::Interval alpha(1.432e-06);

    // Regulation parameters
    ibex::Interval x2_target(10.0); // Desired depth
    ibex::Interval beta(0.04/ibex::Interval::HALF_PI);
    ibex::Interval l1(0.1);
    ibex::Interval l2(0.1);

    ibex::Interval Cf(ibex::Interval::PI*pow(diam/2.0,2));

    // Command
    ibex::Interval A(g*rho/m);
    ibex::Interval B(0.5*rho*Cf/m);

    ibex::Function y(x1, x2, (x1+beta*atan(x2_target-x2)));
    ibex::Function dx1(x1, x2, x3, (-A*(x3-alpha*x2)-B*abs(x1)*x1));
    ibex::Function e(x2, (x2_target-x2));
    ibex::Function D(x2, (1+pow(e(x2), 2)));
    ibex::Function dy(x1, x2, x3, (dx1(x1, x2, x3)-beta*x1/D(x2)));

    ibex::Function u(x1, x2, x3, (1.0/A*(l1*dy(x1, x2, x3)+l2*y(x1, x2)+beta*(2*e(x2)*pow(x1,2)-dx1(x1, x2, x3)*D(x2))/(pow(D(x2),2))-2*B*abs(x1)*x1)+alpha*x1));

    // Evolution function
    ibex::Function f(x1, x2, x3, Return(-A*(x3-alpha*x2)-B*abs(x1)*x1,
                                        x1,
                                        u(x1, x2, x3)));

    DynamicsFunction dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    invariant::MazePPL maze_outer(&dom, &dyn);
//    invariant::MazePPL maze_inner(&dom, &dyn);
    maze_outer.set_widening_limit(5);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    VtkMazePPL vtkMazePPL("Piston");

    for(int i=0; i<28; i++){
        cout << i << endl;
        paving.bisect();
        maze_outer.contract(3*paving.size_active());
        vtkMazePPL.show_maze(&maze_outer);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

//    VtkMaze3D vtkMaze3D("torus", false);
//    vtkMaze3D.show_graph(&paving);
//    vtkMaze3D.show_maze(&maze);

//    IntervalVector position_info(3);
//    position_info[0] = ibex::Interval(0.5);
//    position_info[1] = ibex::Interval(0.5);
//    position_info[2] = ibex::Interval(0.2);
//    vtkMaze3D.show_room_info(&maze, position_info);
}
