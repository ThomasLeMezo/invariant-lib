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
    space[0] = ibex::Interval(0,6);
    space[1] = ibex::Interval(0,6);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Interval k_1(2.9872232), k_2(1), k_3(1), k_4(12.626548), k_5(5.6433885), k_6(0.85914091);

    ibex::Function f(x1, x2, Return(k_1-(k_2+k_3)*x1+k_4*x2-k_5*x1*x2*x2,
                                    k_3*x1-(k_4+k_6)*x2+k_5*x1*x2*x2));

    Dynamics_Function dyn_outer(&f, FWD_BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);

    for(int i=0; i<20; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer);
    v_maze.setProperties(0, 0, 1024, 1024);
//    v_maze.set_enable_cone(false);
    v_maze.show();

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(0.659, 0.665);
    position_info[1] = ibex::Interval(3.513, 3.519);
    v_maze.show_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
