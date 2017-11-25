#include "ibex/ibex_SepFwdBwd.h"
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
    space[0] = Interval(-15, 15);
    space[1] = Interval(-15, 15);
    space[2] = Interval(-1, 50);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    // a=0.432, b=2 and c=4
    // a=0.2, b=0.2, c=8.0
    Interval a = Interval(0.2);
    Interval b = Interval(0.2);
    Interval c = Interval(5.7);

    ibex::Function f(x, y, z, Return(-(y+z),
                                     x+a*y,
                                     b+z*(x-c)));
    Dynamics_Function dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        cout << "-----" << i << "-----" << endl;
        paving.bisect();
        cout << "nb contractions = " << maze.contract() << " - ";
        cout << "paving size = " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VtkMaze3D vtkMaze3D("rossler", true);
    vtkMaze3D.show_graph(&paving);
    vtkMaze3D.show_maze(&maze);

    //    IntervalVector position_info(2);
    //    position_info[0] = Interval(-1.7);
    //    position_info[1] = Interval(1);
    //    v_maze.get_room_info(&maze, position_info);

}
