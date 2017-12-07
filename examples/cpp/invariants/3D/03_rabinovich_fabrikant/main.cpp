#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>
#include "vtkMaze3D.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x, y, z;

    IntervalVector space(3);
    space[0] = ibex::Interval(-4, 2);
    space[1] = ibex::Interval(-2, 2);
    space[2] = ibex::Interval(-2, 3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
//    Interval gamma = ibex::Interval(0.87);
//    Interval alpha = ibex::Interval(1.1);

    ibex::Interval gamma = ibex::Interval(0.1);
    ibex::Interval alpha = ibex::Interval(0.14);


    ibex::Function f(x, y, z, Return(y*(z-1+pow(x,2))+gamma*x,
                                     x*(3.0*z+1-pow(x,2))+gamma*y,
                                     -2*z*(alpha+x*y)));
    Dynamics_Function dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<20; i++){
        cout << "-----" << i << "-----" << endl;
        paving.bisect();
        cout << "nb contractions = " << maze.contract() << " - ";
        cout << "paving size = " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VtkMaze3D vtkMaze3D("rabinovich-fabrikant", true);
    vtkMaze3D.show_graph(&paving);
    vtkMaze3D.show_maze(&maze);

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-1.7);
    //    position_info[1] = ibex::Interval(1);
    //    v_maze.get_room_info(&maze, position_info);

}
