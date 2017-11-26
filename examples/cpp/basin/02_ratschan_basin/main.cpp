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

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x(2);

    IntervalVector space(2);
    space[0] = ibex::Interval(-1,2);
    space[1] = ibex::Interval(-1,1);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subPaving(space);
    invariant::Domain<> dom_outer(&subPaving, invariant::Domain<>::FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.1;
    Function f_sep_outer(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_outer.set_sep_output(&s_outer);

    invariant::Domain<> dom_inner(&subPaving, invariant::Domain<>::FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    Function f_sep_inner(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
    ibex::Function f_outer(x, Return(-(-4*pow(x[0], 3)+6*pow(x[0], 2)-2*x[0]),
                                     -(-2*x[1])));
    Dynamics_Function dyn_outer(&f_outer, Dynamics::FWD);

    ibex::Function f_inner(x, Return((-4*pow(x[0], 3)+6*pow(x[0], 2)-2*x[0]),
                                     (-2*x[1])));
    Dynamics_Function dyn_inner(&f_inner, Dynamics::BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.init();
    maze_inner.init();
    for(int i=0; i<15; i++){
        subPaving.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << subPaving.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << subPaving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << subPaving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "red[]");

//    VibesMaze v_maze2("graph2", &subPaving, &maze_inner, VibesMaze::VIBES_MAZE_INNER);
//    v_maze2.setProperties(0, 0, 512, 512);
//    v_maze2.show();


//    VibesMaze v_maze_inner("SmartSubPaving inner", &subPaving, &maze_inner);
//    v_maze_inner.setProperties(0, 0, 512, 512);
//    v_maze_inner.show();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.1, 0.0);
//    position_info[1] = ibex::Interval(0.16);
//    v_maze_inner.show_room_info(&maze_inner, position_info);

    vibes::endDrawing();

}
