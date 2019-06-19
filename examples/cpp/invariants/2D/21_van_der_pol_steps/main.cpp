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
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(true);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, -Return(x2,
                                     (1.0*(1.0-pow(x1, 2))*x2-x1)));
    ibex::Function f2(x1, x2, -Return(x2,
                                      (1.0*(1.0-pow(x1, 2))*x2-x1)));
    DynamicsFunction dyn_outer(&f, FWD); // Duplicate because of simultaneous access of f (semaphore on DynamicsFunction)
    DynamicsFunction dyn_inner(&f2, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
    std::string file = "/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/";
    file += "van_der_pol_largest_positive_invariant";

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(int i=0; i<15; i++){
        cout << i << endl;

        subpaving.bisect();
        maze_outer.contract();
        maze_inner.contract();

        v_maze.drawBox(space, "white[white]");
        v_maze.show();
        vibes::saveImage(file + to_string(i) + ".svg");
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::endDrawing();

}
