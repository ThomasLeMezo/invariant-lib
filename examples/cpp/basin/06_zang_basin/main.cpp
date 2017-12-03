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

// Note : a integration approach for the outer approximation is more suitable
// In face it avoid any issue with stability point that may let impossible any
// convergence toward the boundary of the set

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x(2);

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.5;
    r = 0.2;
    Function f_sep(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));

    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);
    SepFwdBwd s_outer(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_outer.set_sep_output(&s_outer);

    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    SepFwdBwd s_inner(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
    ibex::Function f(x, Return(x[1],
                                    (8.0/25.0*pow(x[0],5)-4.0/3.0*pow(x[0],3)+4.0/5.0*x[0])));
    Dynamics_Function dyn(&f, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
//    vibes::beginDrawing();
    double time_start = omp_get_wtime();
    for(int i=0; i<20; i++){
        paving.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("Zang Basin", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "black[red]");

    vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/zang_basin.svg", "Zang Basin");

    vibes::endDrawing();

}
