#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"
#include "booleantreeinter.h"

#include "ibex_SepFwdBwd.h"
#include "ibex_SepNot.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>

#include "spacefunction.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-4.75,4.75);
    space[1] = ibex::Interval(-3.5,6);

//    space[0] = ibex::Interval(-3,3);
//    space[1] = ibex::Interval(-3,3);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    double x1_c, x2_c, r;
    x1_c = -2.0;
    x2_c = 4;
    r = 0.3;
    Function f_sep_outer(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    SepNot s_inner(s_outer); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep_input(&s_inner);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    double xB_c, yB_c, rB;
    xB_c = -1;
    yB_c = 1;
    rB = 3.5;
    ibex::Function f_circle(x1, x2, pow(x1-xB_c, 2)+pow(x2-yB_c, 2)-pow(2, rB));
    SepFwdBwd s_circle(f_circle, LEQ);

    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    DynamicsFunction dyn(&f, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    invariant::BooleanTreeInterIBEX bisection_tree(&maze_outer, &maze_inner);
    paving.set_bisection_tree(&bisection_tree);

    dom_outer.set_sep_zero(&s_circle);
    dom_inner.set_sep_zero(&s_circle);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1048, 1048);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "red", "red");
    v_maze.drawCircle(xB_c, yB_c, rB, "green", "");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-2);
//    position_info[1] = ibex::Interval(3.4);
//    v_maze.show_room_info(&maze_inner, position_info);

    vibes::endDrawing();
}
