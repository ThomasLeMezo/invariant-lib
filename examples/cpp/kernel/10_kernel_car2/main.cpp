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
    double constraint_lb = 0.0; // 0.0
    double constraint_u_lb = -1.5; // -3.0, -2.5, -2, -1.5

    ibex::Interval u(constraint_u_lb, 1.0);


    // Constraints
    Function f_id("x[2]", "(x[0], x[1])");
    ibex::IntervalVector limit1(2);
    limit1[0] = ibex::Interval(constraint_lb, 0.0);
    limit1[1] = ibex::Interval(3.0, 14.0);
    SepFwdBwd s_limit1(f_id, limit1);
    SepNot  s_limit1_not(s_limit1);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);
    dom_outer.set_sep_output(&s_limit1_not);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);
    dom_inner.set_sep_input(&s_limit1);

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

//    VibesMaze v_maze("car_kernel_"+to_string(limit1[0].lb()), &maze_outer, &maze_inner);
    VibesMaze v_maze("car_kernel_u"+to_string(u.lb()), &maze_outer, &maze_inner);
    v_maze.show();
    v_maze.drawBox(limit1, "red[]");
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

    vibes::endDrawing();
}
