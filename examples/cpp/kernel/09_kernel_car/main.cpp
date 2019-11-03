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
#include <ibex/ibex_SepInter.h>
#include <ibex/ibex_SepUnion.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2; // x1 position, x2 velocity

    IntervalVector space(2);
    space[0] = ibex::Interval(-30, 0.0);
    space[1] = ibex::Interval(-0.0, 14.0);

    invariant::SmartSubPaving<> paving(space);

    double c = 0.01;
    ibex::Interval u(-3.0, 1.0);
    int constraint = 1; // 0, 1, 2, 3

    if(constraint>=3)
        u=ibex::Interval(-2.5, 1.0);

    // Constraints
    ibex::IntervalVector min_velocity(2);
    min_velocity[0] = ibex::Interval(-30,-5.0);
    min_velocity[1] = ibex::Interval(0.0, 1.0);
    Function f_id("x[2]", "(x[0], x[1])");
    SepFwdBwd s_min_velocity(f_id, min_velocity);
    SepNot s_min_velocity_not(s_min_velocity);

    ibex::IntervalVector limit1(2);
    limit1[0] = ibex::Interval(-5.0, 0.0);
    limit1[1] = ibex::Interval(3.0, 14.0);
    SepFwdBwd s_limit1(f_id, limit1);

    SepUnion s_inter(s_limit1, s_min_velocity);
    SepNot  s_inter_not(s_inter);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);
    switch (constraint) {
    case 0:
        break;
    case 1:
        dom_outer.set_sep_output(&s_min_velocity_not);
        break;
    default:
        dom_outer.set_sep_output(&s_inter_not);
        break;
    }

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);
    switch (constraint) {
    case 0:
        break;
    case 1:
        dom_inner.set_sep_input(&s_min_velocity);
        break;
    default:
        dom_inner.set_sep_input(&s_inter);
        break;
    }

    // ****** Dynamics Outer ******* //
    ibex::Function f_outer(x1, x2, Return(x2,u-c*abs(x2)*x2));
    DynamicsFunction dyn_outer(&f_outer, BWD);

    // ****** Dynamics Inner ******* //
    ibex::Function f_inner1(x1, x2, -Return(x2,u.lb()-c*abs(x2)*x2));
    ibex::Function f_inner2(x1, x2, -Return(x2,u.ub()-c*abs(x2)*x2));

    vector<Function *> f_list_inner;
    f_list_inner.push_back(&f_inner1);
    f_list_inner.push_back(&f_inner2);
    DynamicsFunction dyn_inner(f_list_inner, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("car_kernel_"+to_string(constraint), &maze_outer, &maze_inner);
//    v_maze.set_enable_cone(true);
    v_maze.show();
    if(constraint>0)
        v_maze.drawBox(min_velocity, "red[]");
    if(constraint>1)
        v_maze.drawBox(limit1, "red[]");
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

    vibes::endDrawing();
}
