#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

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
    space[0] = ibex::Interval(-6,6);
    space[1] = ibex::Interval(-6,6);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep_outer(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_outer.set_sep_output(&s_outer);

    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

//    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep_inner(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
    ibex::Function f_outer(x, Return(-(x[1]),
                           -(-0.5*x[1]-sin(x[0]+0.412)+sin(0.412))));
    ibex::Function f_inner(x, Return((x[1]),
                           (-0.5*x[1]-sin(x[0]+0.412)+sin(0.412))));
    Dynamics_Function dyn_outer(&f_outer, FWD);
    Dynamics_Function dyn_inner(&f_inner, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
//    vibes::beginDrawing();
    double time_start = omp_get_wtime();
    maze_inner.init();
    
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("Synchronous Generator Basin", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "black[red]");

//    VibesMaze v_maze_inner("graph_inner",&maze_inner, VibesMaze::VIBES_MAZE_INNER);
//    v_maze_inner.setProperties(0, 0, 512, 512);
//    v_maze_inner.show();

//    VibesMaze v_maze_outer("graph_outer",&maze_outer, VibesMaze::VIBES_MAZE_OUTER);
//    v_maze_outer.setProperties(0, 0, 512, 512);
//    v_maze_outer.show();

//    VibesMaze v_maze_info("graph_info", &maze_inner);
//    v_maze_info.setProperties(0, 0, 512, 512);
//    v_maze_info.show();
//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-1);
//    position_info[1] = ibex::Interval(2, 4);
//    v_maze_info.setProperties(0, 0, 300, 300);
//    v_maze_info.show_room_info(&maze_inner, position_info);

    vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/synchronous_generator_basin.svg", "Synchronous Generator Basin");

    vibes::endDrawing();

}
