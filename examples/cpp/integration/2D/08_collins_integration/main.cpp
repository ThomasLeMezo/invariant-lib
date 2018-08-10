#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"
#include "ibex_SepUnion.h"
#include "ibex_SepInter.h"
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
    space[0] = ibex::Interval(-4,4);
    space[1] = ibex::Interval(-3.5,6);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = -2.0;
    r = 1.0;

//    x1_c = 0.0;
//    x2_c = 1.0;
//    r = 9.0/100.0;

    Function f_sep_outer(x1, x2, pow(x1, 2)+pow(x2+2, 2)-pow(1.0, 2));
    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT
    Function f_sep_outer_bis(x1, x2, pow(x1-1, 2)+pow(x2+3, 2)-pow(0.1, 2));
    SepFwdBwd s_outer_bis(f_sep_outer_bis, LEQ); // LT, LEQ, EQ, GEQ, GT)
    SepUnion sep_u(s_outer, s_outer_bis);

    dom_outer.set_sep_input(&sep_u);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_DOOR);

    SepNot sep_inner(sep_u);
    dom_inner.set_sep_input(&sep_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x1, x2, Return((2*x1-x1*x2),
                                    (2*pow(x1,2)-x2)));
    DynamicsFunction dyn_outer(&f, FWD);
    DynamicsFunction dyn_inner(&f, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
//    omp_set_num_threads(1);
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "red[]");
    v_maze.drawCircle(1, -3, 0.1, "red[]");
    v_maze.drawCircle(0.0, 1.0, 9.0/100.0, "black[green]");

//    VibesMaze v_maze_inner("SmartSubPaving", &maze_inner);
//    v_maze_inner.setProperties(0, 0, 1024, 1024);
//    v_maze_inner.show();
//    v_maze_inner.drawCircle(x1_c, x2_c, r, "red[]");
//    v_maze_inner.drawCircle(1, -3, 0.1, "red[]");
//    v_maze_inner.drawCircle(0.0, 1.0, 9.0/100.0, "black[green]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.001, 0.001);
//    position_info[1] = ibex::Interval(-3.02);
//    v_maze.show_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
