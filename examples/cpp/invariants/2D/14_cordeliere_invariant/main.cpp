#include "ibex/ibex_SepFwdBwd.h"
#include "ibex/ibex_SepInter.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
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
    space[0] = Interval(-10, 10);
    space[1] = Interval(-10, 10);

    // ****** Domain ******* //
    SmartSubPaving paving(space);

    invariant::Domain dom_outer(&paving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    invariant::Domain dom_inner(&paving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

//    Function f_sep(x1, x2, sqr(x1-1.78129)+sqr(x2-1.78129)-0.5);
//    Function f_sep2(x1, x2, sqr(x1+1.78129)+sqr(x2+1.78129)-0.5);
//    Function f_sep3(x1, x2, sqr(x1)+sqr(x2)-0.5);
//    SepFwdBwd sep(f_sep, GEQ);
//    SepFwdBwd sep2(f_sep2, GEQ);
//    SepFwdBwd sep3(f_sep3, GEQ);
//    SepInter sep_i(sep, sep2, sep3);
//    dom.set_sep(&sep_i);

    // ****** Dynamics ******* //
    double h0 = -9;

    ibex::Function h(x1, x2, 2.0*exp((sqr(x1+2)+sqr(x2+2))/-10.0) + 2.0*exp((sqr(x1-2)+sqr(x2-2))/-10.0) - 10.0);
    ibex::Function hdiff(h, Function::DIFF);
    ibex::Function psi(x1, x2, tanh(h(x1, x2)-h0)+Interval::PI/2.0);
    ibex::Function f(x1, x2, Return((hdiff(x1, x2)[0]*cos(psi(x1, x2))-hdiff(x1, x2)[1]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0])),
                                    (hdiff(x1, x2)[1]*cos(psi(x1, x2))+hdiff(x1, x2)[0]*sin(psi(x1, x2)))/sqrt(sqr(hdiff(x1, x2)[1])+sqr(hdiff(x1, x2)[0]))));

    Dynamics_Function dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    Maze maze_outer(&dom_outer, &dyn);
    Maze maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(int i=0; i<20; i++){
        cout << i << endl;
        paving.bisect();
        cout << maze_outer.contract() << endl;
        cout << maze_inner.contract() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
//    v_maze.drawCircle(0, 0, 0.5, "black[red]");
//    v_maze.drawCircle(1.78129, 1.78129, 0.5, "black[red]");
//    v_maze.drawCircle(-1.78129, -1.78129, 0.5, "black[red]");

//    IntervalVector position_info(2);
//    position_info[0] = Interval(2.22);
//    position_info[1] = Interval(1.97);
//    v_maze.get_room_info(&maze, position_info);
//    v_maze.show_room_info(&maze, position_info);

    vibes::endDrawing();
}
