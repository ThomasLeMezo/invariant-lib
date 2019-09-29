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
    ibex::Variable x(2);

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);

    ibex::IntervalVector box1(2), box2(2);
    box1[0] = ibex::Interval(0.6, 1.4);
    box1[1] = ibex::Interval(-0.4, 0.4);
    box2[0] = ibex::Interval(-1.0, -0.6);
    box2[1] = ibex::Interval(0.3, 0.7);

    Function f_id("x[2]", "(x[0], x[1])");
    SepFwdBwd s_outer_box1(f_id, box1);
    SepFwdBwd s_outer_box2(f_id, box2);
    SepUnion s_outer(s_outer_box1, s_outer_box2);
    SepNot s_inner(s_outer);

    // Domains

    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);
    dom_outer.set_sep_input(&s_outer);

    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
    ibex::Function f(x, Return(x[1],
                                    (1.0*(1.0-pow(x[0], 2))*x[1]-x[0])));
    DynamicsFunction dyn(&f, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
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
    VibesMaze v_maze("vdp_basin_2", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
    v_maze.show();

    v_maze.drawBox(box1, "black[red]");
    v_maze.drawBox(box2, "black[red]");
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");
    vibes::endDrawing();

}
