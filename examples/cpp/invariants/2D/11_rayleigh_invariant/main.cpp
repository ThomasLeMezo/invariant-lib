#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex/ibex_SepFwdBwd.h"

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
    space[0] = Interval(-2,2);
    space[1] = Interval(-2,2);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_DOOR);

    Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(0.3, 2));
    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                    -1.0*(x1+pow(x2,3)-x2)));
    Dynamics_Function dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<18; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - ";
        cout << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-2);
//    position_info[1] = Interval(-2.9);
//    v_maze.get_room_info(&maze, position_info);
//    v_maze.show_room_info(&maze, position_info);

    vibes::endDrawing();

}
