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
    ibex::Variable x, y, z;

    IntervalVector space(3);
    space[0] = ibex::Interval(-1, 1);
    space[1] = ibex::Interval(-1, 1);
    space[2] = ibex::Interval(-1, 1);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);

    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    Function f_sep_outer(x, y, z, pow(x, 2)+pow(y, 2) + pow(z, 2) - pow(0.1, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep(&s_outer);

    ibex::Function f(x, y, z, Return(ibex::Interval(1.0, 1.1), ibex::Interval(1.0, 1.1), ibex::Interval(-0.1,0.1)));
    DynamicsFunction dyn_outer(&f, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze(&dom_outer, &dyn_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze.contract();
    for(int i=0; i<10; i++){
        paving.bisect();
        cout << i << " outer - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    VtkMaze3D vtkMaze3D("wrapping", false);
    vtkMaze3D.show_graph(&paving);
    vtkMaze3D.show_maze(&maze, "outer");

    IntervalVector position(3);
    position[0] = ibex::Interval(0);
    position[1] = ibex::Interval(-0.1);
    position[2] = ibex::Interval(-0.1);
    vtkMaze3D.show_room_info(&maze, position);

}
