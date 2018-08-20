#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

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
    space[0] = ibex::Interval(-1, 1);
    space[1] = ibex::Interval(-1, 1);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);

    double x1_0, x2_0, r;
    x1_0 = 0.0;
    x2_0 = 0.0;
    r = 0.1;

    Function f_sep_outer(x1, x2, pow(x1-x1_0, 2)+pow(x2-x2_0, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x1, x2, Return(ibex::Interval(-1, 1),
                                    ibex::Interval(-1, 1)));

    DynamicsFunction dyn_outer(&f, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.contract();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-2);
    //    position_info[1] = ibex::Interval(-2);
    //    v_maze_outer.get_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
