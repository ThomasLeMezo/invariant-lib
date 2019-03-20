#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
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
    space[0] = -ibex::Interval::PI  | ibex::Interval::PI;
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics ******* //

    ibex::Interval b(0.5);
    ibex::Interval m(1);
    ibex::Interval R(1);
    ibex::Interval g(9.81);

    ibex::Function f(x1, x2, Return(x2,
                                    (-b/(m*pow(R,2))*x2-g/R*sin(x1))));
    DynamicsFunction dyn(&f, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);

    for(int i=0; i<14; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);
    v_maze.show();

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(-2.5);
    position_info[1] = ibex::Interval(2.9);
//    v_maze.get_room_info(&maze, position_info);
//    v_maze.show_room_info(&maze, position_info);

    std::string file = "/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/";
    file += "pendulum_invariant.svg";
    vibes::saveImage(file);
    vibes::endDrawing();

}
