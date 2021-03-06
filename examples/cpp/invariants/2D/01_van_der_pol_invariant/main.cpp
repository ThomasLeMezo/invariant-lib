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

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
    DynamicsFunction dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);
    maze.set_enable_contract_vector_field(true);

    // ******* Display ********* //
    vibes::beginDrawing();
    VibesMaze v_maze("VanDerPolInvariant", &maze);
    v_maze.setProperties(0, 0, 1024, 1024);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    
//    omp_set_num_threads(1);
    for(int i=0; i<12; i++){
        cout << i << endl;

        double time_local = omp_get_wtime();

        paving.bisect();
        maze.contract();

        // Stat
        double v_outer =  v_maze.get_volume();
        cout << "time " << omp_get_wtime() - time_local << endl;
        cout << "volume outer = " << v_outer << endl;
    }
    cout << "TIME TOTAL = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    v_maze.show();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(2.94);
//    position_info[1] = ibex::Interval(-2.82);
//    v_maze.show_room_info(&maze, position_info);

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-1);
//    position_info[1] = ibex::Interval(1);
//    v_maze.get_room_info(&maze, position_info);
//    v_maze.show_room_info(&maze, position_info);

//    position_info[0] = ibex::Interval(0);
//    position_info[1] = ibex::Interval(-1);
//    v_maze.get_room_info(&maze, position_info);
    vibes::endDrawing();

}
