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

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = Interval(-3,3);
    space[1] = Interval(-3,3);

    // ****** Domain ******* //
    SmartSubPaving subpaving(space);

    invariant::Domain dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    invariant::Domain dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn_outer(&f, FWD_BWD);
    Dynamics_Function dyn_inner(&f, FWD);

    // ******* Maze ********* //
    Maze maze_outer(&dom_outer, &dyn_outer);
    Maze maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_inner.init();
    for(int i=0; i<15; i++){
        subpaving.bisect();
        cout << i << " - " << maze_outer.contract() << " - ";
        cout << i << " - " << maze_inner.contract() << " - ";
        cout << subpaving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(0);
//    position_info[1] = Interval(1);
//    v_maze.get_room_info(&maze, position_info);

    IntervalVector position_info(2);
    position_info[0] = Interval(-2.5);
    position_info[1] = Interval(2.9);
//    v_maze.get_room_info(&maze, position_info);
//    v_maze.show_room_info(&maze, position_info);

//    position_info[0] = Interval(0);
//    position_info[1] = Interval(-1);
//    v_maze.get_room_info(&maze, position_info);
    vibes::endDrawing();

}
