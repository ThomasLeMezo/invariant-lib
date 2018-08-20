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
#include "vtkMaze3D.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x(3);

    IntervalVector space(3);
    space[0] = ibex::Interval(-1, 1);
    space[1] = ibex::Interval(-1, 1);
    space[2] = ibex::Interval(-1, 1);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
//    Interval b = ibex::Interval(0.32899);
//    Interval b = ibex::Interval(0.208186);

    ibex::Function f(x, Return(ibex::Interval(1),
                                     x[2],
                                     -x[1]-0.1*x[2]));
    DynamicsFunction dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);

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

    VtkMaze3D vtkMaze3D("thomas",  true);
    vtkMaze3D.show_graph(&paving);
    vtkMaze3D.show_maze(&maze);

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-1.7);
    //    position_info[1] = ibex::Interval(1);
    //    v_maze.get_room_info(&maze, position_info);

}
