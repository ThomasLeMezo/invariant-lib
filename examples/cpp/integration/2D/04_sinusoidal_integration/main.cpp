#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex/ibex_SepFwdBwd.h"
#include "ibex/ibex_SepInter.h"
#include "ibex/ibex_SepUnion.h"

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
    space[1] = ibex::Interval(-3,3);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, invariant::Domain<>::FULL_WALL);

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
    invariant::Domain<> dom_inner(&paving, invariant::Domain<>::FULL_DOOR);

    SepFwdBwd s_inner_x1(f_sep_outer_x1, GEQ); // LT, LEQ, EQ, GEQ, GT
    SepFwdBwd s_inner_x2(f_sep_outer_x2, GEQ); // LT, LEQ, EQ, GEQ, GT
    SepUnion s_inner(s_inner_x1, s_inner_x2);
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x1, x2, Return(ibex::Interval(1.0),
                                    -sin(x2)));
    Dynamics_Function dyn_outer(&f, Dynamics::FWD);
    Dynamics_Function dyn_inner(&f, Dynamics::FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.contract();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    vibes::drawBox(ibex::Interval(0, 0.5), ibex::Interval(-2, 2), "red[]");

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(0.01);
    position_info[1] = ibex::Interval(1.99);
    v_maze.show_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
