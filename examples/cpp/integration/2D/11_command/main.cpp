#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"
#include "booleantreeinter.h"
#include "spacefunction.h"

#include "ibex_SepFwdBwd.h"
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
    space[0] = ibex::Interval(-0.5, 10);
    space[1] = ibex::Interval(-3, 3);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    ibex::IntervalVector initial_condition(2);
    initial_condition[0] = ibex::Interval(0, 1);
    initial_condition[1] = ibex::Interval(-0.5, 0.5);
    Function f_id(x1, x2, Return(x1, x2));
    SepFwdBwd s_outer(f_id, initial_condition); // LT, LEQ, EQ, GEQ, GT
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
    // Function Dead Reckoning
    ibex::Function f_dead_reckoning(x1, x2, Return(1.0+0.0*x1, 0.05 + ibex::Interval(-0.1, 0.1)+0*x1));

    // Function Follow line
    ibex::Function f_line(x1, x2, Return(1+0*x1, -sin(x2)));

    SpaceFunction sf = SpaceFunction();

    Function s_dead_reckoning_f(x1, x2, x1-5);
    SepFwdBwd s_dead_reckoning(s_dead_reckoning_f, LT); // LT, LEQ, EQ, GEQ, GT
    sf.push_back(&f_dead_reckoning, &s_dead_reckoning);

    Function s_line_f(x1, x2, x1-5);
    SepFwdBwd s_line(s_line_f, GEQ); // LT, LEQ, EQ, GEQ, GT
    sf.push_back(&f_line, &s_line);

//    ibex::Function f(x1, x2, Return(heaviside()));

    Dynamics_Function dyn(&sf, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    invariant::BooleanTreeInterIBEX bisection_tree(&maze_outer, &maze_inner);
    paving.set_bisection_tree(&bisection_tree);

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
    v_maze.drawBox(initial_condition, "red[]");
//    v_maze.drawCircle(x1_c, x2_c, r, "red[red]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-2);
//    position_info[1] = ibex::Interval(3.4);
//    v_maze.show_room_info(&maze_inner, position_info);

    vibes::endDrawing();
}
