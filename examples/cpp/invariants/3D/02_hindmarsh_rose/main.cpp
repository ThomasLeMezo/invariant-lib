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
    space[0] = ibex::Interval(-5,5);
    space[1] = ibex::Interval(-20,10);
    space[2] = ibex::Interval(-4,50);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Interval a = ibex::Interval(1.0);
    ibex::Interval b = ibex::Interval(3.0);
    ibex::Interval c = ibex::Interval(1.0);
    ibex::Interval d = ibex::Interval(5.0);
    ibex::Interval r = ibex::Interval(1e-3);
    ibex::Interval s = ibex::Interval(4.0);
    ibex::Interval xR = ibex::Interval(-8.0/5.0);
    ibex::Interval I = ibex::Interval(5.0);

    ibex::Function f(x, y, z, Return(y+(-a*pow(x,3)+b*pow(x,2))-z+I,
                                        c-d*pow(x,2)-y,
                                        r*(s*(x-xR)-z)));
    Dynamics_Function dyn(&f, FWD_BWD);

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

    VtkMaze3D vtkMaze3D("hindmarsh-rose", true);
    vtkMaze3D.show_graph(&paving);
    vtkMaze3D.show_maze(&maze);

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-1.7);
//    position_info[1] = ibex::Interval(1);
//    v_maze.get_room_info(&maze, position_info);

}
