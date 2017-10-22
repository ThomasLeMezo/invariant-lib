#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"

#include "ibex/ibex_SepFwdBwd.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

#include "vtk_graph.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x, y, z;

    IntervalVector space(3);
    space[0] = Interval(-1, 1);
    space[1] = Interval(-1, 1);
    space[2] = Interval(-1, 1);

    SmartSubPaving paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&paving, FULL_WALL);

    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    Function f_sep_outer(x, y, z, pow(x, 2)+pow(y, 2) + pow(z, 2) - pow(0.1, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep(&s_outer);

    ibex::Function f(x, y, z, Return(Interval(1.0, 1.1), Interval(1.0, 1.1), Interval(-0.1,0.1)));
    Dynamics_Function dyn_outer(&f, FWD);

    // ******* Mazes ********* //
    Maze maze(&dom_outer, &dyn_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze.contract();
    for(int i=0; i<10; i++){
        paving.bisect();
        cout << i << " outer - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    Vtk_Graph vtk_graph("wrapping", &paving, false);
    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze, "outer");

    IntervalVector position(3);
    position[0] = Interval(0);
    position[1] = Interval(-0.1);
    position[2] = Interval(-0.1);
    vtk_graph.show_room_info(&maze, position);

}
