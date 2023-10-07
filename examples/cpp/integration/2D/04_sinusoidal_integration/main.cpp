#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"
#include "ibex_SepInter.h"
#include "ibex_SepUnion.h"
#include "ibex_SepNot.h"

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
    space[1] = ibex::Interval(-3,3.1);

    invariant::SmartSubPaving<ExpBox> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<ExpBox> dom_outer(&paving, FULL_WALL);

    double x1_max, x2_norm;
    x1_max = 0.5;
    x2_norm = 2.0;

    Function f_sep_outer_x1(x1, x2, x1-x1_max);
    Function f_sep_outer_x2(x1, x2, pow(x2, 2)-pow(x2_norm, 2));

    SepFwdBwd s_outer_x1(f_sep_outer_x1, LEQ); // LT, LEQ, EQ, GEQ, GT
    SepFwdBwd s_outer_x2(f_sep_outer_x2, LEQ); // LT, LEQ, EQ, GEQ, GT
    SepInter s_outer(s_outer_x1, s_outer_x2);

    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<ExpBox> dom_inner(&paving, FULL_DOOR);

    SepNot s_inner(s_outer);
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x1, x2, Return(ibex::Interval(1.0),
                                    -sin(x2)));
    DynamicsFunction dyn(&f, FWD);

    // ******* Mazes ********* //
    invariant::Maze<ExpBox> maze_outer(&dom_outer, &dyn);
    invariant::Maze<ExpBox> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << endl;
        maze_inner.contract();
        maze_outer.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMazeEXP v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    //v_maze.drawBox(ibex::Interval(0, 0.5), ibex::Interval(-2, 2), "red", "");

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(4);
    position_info[1] = ibex::Interval(1);
//    v_maze.show_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
