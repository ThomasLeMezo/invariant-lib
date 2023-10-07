#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain *******
    invariant::SmartSubPaving<ExpBox> paving(space);


    invariant::Domain<ExpBox> dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 1.0;
    x2_c = 1.0;
    r = 0.2;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LT); // LT, LEQ, EQ, GEQ, GT)
    dom_outer.set_sep(&s);

    invariant::Domain<ExpBox> dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    Function f_sep_inner(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_inner.set_sep(&s_inner);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    DynamicsFunction dyn(&f, FWD);

    // ******* Maze *********
    invariant::Maze<ExpBox> maze_outer(&dom_outer, &dyn);
    invariant::Maze<ExpBox> maze_inner(&dom_inner, &dyn);

    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMazeEXP v_maze("vdp_fwdEXP", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(true);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "red", "red");

    v_maze.saveImage("./", ".svg");

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(0.62);
    position_info[1] = ibex::Interval(-1.525);
    v_maze.show_room_info(&maze_outer, position_info);

//    IntervalVector pave_in(2);
//    pave_in[0] = ibex::Interval(2.67188, 2.71875);
//    pave_in[1] = ibex::Interval(2.0625, 2.15625);
//    IntervalVector pave_out(pave_in);
//    s.separate(pave_in, pave_out);
//    cout << "pave_in = " << pave_in << " pave_out = " << pave_out << endl;
//    vibes::drawBox(pave_out, "black[]");

}
