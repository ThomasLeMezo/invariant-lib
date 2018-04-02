#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
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
    Dynamics_Function dyn(&f, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("Van Der Pol 2 Basin", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();

//    VibesMaze v_maze_inner("graph_inner",&maze_inner, VibesMaze::VIBES_MAZE_INNER);
//    v_maze_inner.setProperties(0, 0, 512, 512);
//    v_maze_inner.show();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-1);
//    position_info[1] = ibex::Interval(0.68);
//    v_maze.show_room_info(&maze_inner, position_info);

//    position_info[0] = ibex::Interval(-0.34);
//    position_info[1] = ibex::Interval(1.34);
//    v_maze.get_room_info(&maze_inner, position_info);

////    v_maze.drawCircle(x1_c, x2_c, r, "black[red]");
    v_maze.drawBox(box1, "red[]");
    v_maze.drawBox(box2, "red[]");
    vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/van_der_pol2_basin.svg", "Van Der Pol 2 Basin");
    vibes::endDrawing();

}
