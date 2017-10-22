#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibes_graph.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>
#include "vtk_graph.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x, y, z;

    IntervalVector space(3);
    space[0] = Interval(-5,5);
    space[1] = Interval(-20,10);
    space[2] = Interval(-4,50);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    Interval a = Interval(1.0);
    Interval b = Interval(3.0);
    Interval c = Interval(1.0);
    Interval d = Interval(5.0);
    Interval r = Interval(1e-3);
    Interval s = Interval(4.0);
    Interval xR = Interval(-8.0/5.0);
    Interval I = Interval(5.0);

    ibex::Function f(x, y, z, Return(y+(-a*pow(x,3)+b*pow(x,2))-z+I,
                                        c-d*pow(x,2)-y,
                                        r*(s*(x-xR)-z)));
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

    Vtk_Graph vtk_graph("hindmarsh-rose", &paving, true);
    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze);

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-1.7);
//    position_info[1] = Interval(1);
//    v_graph.get_room_info(&maze, position_info);

}
