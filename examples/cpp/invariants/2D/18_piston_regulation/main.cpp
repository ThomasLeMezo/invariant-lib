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
    space[0] = ibex::Interval(-1, 1);
    space[1] = ibex::Interval(-1, 1);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Interval e1(0);
    ibex::Interval e2(0);
    ibex::Interval beta(0.01/M_PI_2);
    ibex::Interval B(2.61);
    ibex::Function f(x1, x2, Return(-beta*atan(x2)-x1*B,
                                    x1+e1));

    ibex::Function f1(x1, x2, Return(-beta*atan(x2)+e2.lb(),
                                     x1+e1));
//    ibex::Function f2(x1, x2, Return(-beta*atan(x2)+e2.ub(),
//                                     x1+e1));

//    ibex::Function f3(x1, x2, Return(-atan(x2)+e2.lb(),
//                                     x1+e1));
//    ibex::Function f4(x1, x2, Return(-atan(x2)+e2.ub(),
//                                     x1+e1));
//    ibex::Function f3(x1, x2, -Return(atan(x2)+e2.lb(),
//                                     x1+e1.ub()));
//    ibex::Function f4(x1, x2, -Return(atan(x2)+e2.ub(),
//                                     x1+e1.ub()));
    std::vector<ibex::Function*> f_list_inner = {&f1/*, &f2*/};

    DynamicsFunction dyn_outer(&f, FWD_BWD); // Duplicate because of simultaneous access of f (semaphore on DynamicsFunction)

    DynamicsFunction dyn_inner(f_list_inner, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(int i=0; i<15; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
//        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subpaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(true);
    v_maze.show();


    vibes::endDrawing();

}
