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
    space[0] = ibex::Interval(-4,4);
    space[1] = ibex::Interval(-4,4);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_DOOR);

//    Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(0.3, 2));
//    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
//    dom.set_sep(&s);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x1-(x1+x2)*(pow(x1,2)+pow(x2,2)),
                                    x2+(x1-x2)*(pow(x1,2)+pow(x2,2))));
    DynamicsFunction dyn(&f, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);

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
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-2);
//    position_info[1] = ibex::Interval(-2.9);
//    v_maze.get_room_info(&maze, position_info);
//    v_maze.show_room_info(&maze, position_info);

    vibes::endDrawing();

}
