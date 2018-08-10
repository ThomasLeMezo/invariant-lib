#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
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
    space[0] = ibex::Interval(-10, 6);
    space[1] = ibex::Interval(-6, 10);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    ibex::IntervalVector initial_condition(2);
    initial_condition[0] = ibex::Interval(-8, -7);
    initial_condition[1] = ibex::Interval(7, 8);
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
    ibex::Interval yaw_dr_iv = -ibex::Interval::HALF_PI/2.0 + ibex::Interval(-0.3, 0.3);
    ibex::Function f_dead_reckoning(x1, x2, Return(cos(yaw_dr_iv), sin(yaw_dr_iv)));

    // Function Follow line
    double h0 = -9;
    ibex::Interval sensor_error(-0.01, 0.01);
    ibex::Function h(x1, x2, 2.0*exp((sqr(x1+2)+sqr(x2+2))/-10.0) + 2.0*exp((sqr(x1-2)+sqr(x2-2))/-10.0) - 10.0);
    ibex::Function hdiff(h, Function::DIFF);
    ibex::Function psi(x1, x2, tanh(h(x1, x2)-h0+sensor_error)+ibex::Interval::PI/2.0);
    ibex::Function f_bathy(x1, x2, Return((hdiff(x1, x2)[0]*cos(psi(x1, x2))-hdiff(x1, x2)[1]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0])),
                                    (hdiff(x1, x2)[1]*cos(psi(x1, x2))+hdiff(x1, x2)[0]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0]))));


    SpaceFunction sf = SpaceFunction();

//    ibex::IntervalVector box_bathy(2);
//    box_bathy[0] = ibex::Interval(-6, 6);
//    box_bathy[1] = ibex::Interval(-6, 6);
    //    SepFwdBwd s_bathy(f_id, box_bathy);

    double h0_threshold = -9.5;
    ibex::Function f_sep_bathy(x1, x2, (h(x1, x2)-h0_threshold+sensor_error));
    SepFwdBwd s_bathy(f_sep_bathy, GEQ);

    SepNot s_dead_reckoning(s_bathy);

    sf.push_back(&f_bathy, &s_bathy);
    sf.push_back(&f_dead_reckoning, &s_dead_reckoning);

    DynamicsFunction dyn(&sf, FWD);

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
