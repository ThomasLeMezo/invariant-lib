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
    space[0] = Interval(-4.75,4.75);
    space[1] = Interval(-3.5,6);

    SmartSubPaving paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&paving, FULL_WALL);

    double x1_c, x2_c, r;
//    x1_c = 3.0;
//    x2_c = 2.0;
    x1_c = -2.0;
    x2_c = 4;
    r = 0.3;

    Function f_sep_outer(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain dom_inner(&paving, FULL_DOOR);

    Function f_sep_inner(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x1, x2, Return(x2,
                                        (1.0*(1.0-pow(x1, 2))*x2-x1)));
//    ibex::Function f(x1, x2, Return(x2,
//                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+Interval(-0.3, 0.3)));
    Dynamics_Function dyn_outer(&f, FWD);
    Dynamics_Function dyn_inner(&f, FWD);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer);
    Maze maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.init();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
//    VibesMaze v_maze("SmartSubPaving", &maze_inner);
    v_maze.setProperties(0, 0, 1048, 1048);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "red[]");

    IntervalVector position_info(2);
    position_info[0] = Interval(-2.28);
    position_info[1] = Interval(3.89);
//    v_maze.get_room_info(&maze_inner, position_info);
//    v_maze.show_room_info(&maze_inner, position_info);

//    IntervalVector pave_in(2);
//    pave_in[0] = Interval(-2.375, -1.78125);
//    pave_in[1] = Interval(3.625, 4.8125);
//    IntervalVector pave_out(pave_in);
//    vibes::drawBox(pave_out, "black[]");
//    s_inner.separate(pave_in, pave_out);
//    cout << "pave_in = " << pave_in << " pave_out = " << pave_out << endl;
//    vibes::drawBox(pave_in, "green[]");

    vibes::endDrawing();

}
