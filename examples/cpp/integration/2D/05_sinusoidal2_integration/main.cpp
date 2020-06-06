#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

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
    ibex::Variable x, t;

    IntervalVector space(2);
    space[0] = ibex::Interval(-0.1,8);
    space[1] = ibex::Interval(-3,3);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);

    double x_0, t_0, r;
    x_0 = 1.0;
    t_0 = 1.0;
    r = 1.0;

    Function f_sep_outer(t, x, pow(x-x_0, 2)+pow(t-t_0, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_DOOR);

    SepFwdBwd s_inner(f_sep_outer, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Interval a = ibex::Interval(10.0);
    ibex::Function f(t, x, Return(ibex::Interval(1.0),
                                    -a*x+(-1+a)*sin(t)+(1+a)*cos(t)));
    ibex::Function f2(t, x, -Return(ibex::Interval(1.0),
                                    -a*x+(-1+a)*sin(t)+(1+a)*cos(t)));

    DynamicsFunction dyn_outer(&f, FWD);
    DynamicsFunction dyn_inner(&f2, BWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.contract();
    for(int i=0; i<18; i++){
        paving.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
        v_maze.drawCircle(1.0, 1.0, 1, "black", "red");

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-2);
    //    position_info[1] = ibex::Interval(-2);
    //    v_maze_outer.get_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
