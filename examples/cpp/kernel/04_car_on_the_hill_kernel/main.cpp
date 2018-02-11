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

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
//    space[0] = ibex::Interval(-1.0, 13.0);
//    space[1] = ibex::Interval(-16, 16);
    space[0] = ibex::Interval(-1.0, 13.0);
    space[1] = ibex::Interval(-6, 6);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_DOOR);

    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_WALL);

    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer ******* //
//    ibex::Function f_outer(x1, x2, Return(x2,
//                                     (-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + ibex::Interval(-0.5, 0.5))));
    ibex::Function f_outer(x1, x2, Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-0.5, 0.5)));

    Dynamics_Function dyn_outer(&f_outer, FWD_BWD);

    // ****** Dynamics Inner ******* //
//    ibex::Function f_inner1(x1, x2, Return(-x2,
//                                     -(-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + ibex::Interval(-0.5))));
//    ibex::Function f_inner2(x1, x2, Return(-x2,
//                                     -(-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + ibex::Interval(0.5))));
    ibex::Function f_inner1(x1, x2, Return(x2,
                                     (-9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-0.5))));
    ibex::Function f_inner2(x1, x2, Return(x2,
                                     (-9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(0.5))));

    vector<Function *> f_list_inner;
    f_list_inner.push_back(&f_inner1);
    f_list_inner.push_back(&f_inner2);
    Dynamics_Function dyn_inner(f_list_inner, BWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    vibes::beginDrawing();
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
        maze_inner.contract();
        maze_outer.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("Kernel car on the hill"/*, &maze_outer*/, &maze_inner);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(12.24);
    position_info[1] = ibex::Interval(-0.24);
    v_maze.setProperties(600, 0, 512, 512);
    v_maze.show_room_info(&maze_inner, position_info);

    vibes::endDrawing();
}
