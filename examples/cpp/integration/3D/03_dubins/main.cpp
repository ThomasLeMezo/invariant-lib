#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"

#include "ibex_SepFwdBwd.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

#include "vtkMaze3D.h"

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

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);

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

    // ****** Domain Inner ******* //
//    invariant::Domain<> dom_inner(&paving, FULL_DOOR);

//    SepFwdBwd s_inner(f_sep_outer, GEQ); // LT, LEQ, EQ, GEQ, GT
//    dom_inner.set_sep(&s_inner);

//    dom_inner.set_border_path_in(true);
//    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
        ibex::Function f(x, y, theta, Return(v*cos(theta),
                                            v*sin(theta),
                                            u));
//    ibex::Function f(x1, x2, Return(x2,
//                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+Interval(-0.3, 0.3)));
    DynamicsFunction dyn_outer(&f, FWD);
//    DynamicsFunction dyn_inner(&f, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
//    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.contract();
    for(int i=0; i<23; i++){
        paving.bisect();
//        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
        cout << i << " outer - " << endl;
        maze_outer.contract();
        cout << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VtkMaze3D vtkMaze3D("dubins",  true);
    vtkMaze3D.show_graph(&paving);
    vtkMaze3D.show_maze(&maze_outer, "outer");
//    vtkMaze3D.show_maze(&maze_inner, "inner");

}
