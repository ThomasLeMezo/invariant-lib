#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibes_graph.h"

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
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = Interval(-2, 13.0);
    space[1] = Interval(-3, 5);

    SmartSubPaving paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&paving, FULL_WALL);

    double x1_c, x2_c, r;
//    x1_c = 3.0;
//    x2_c = 2.0;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 1.0;

    IntervalVector box_init(2);
    box_init[0] = Interval(-1, 1);
    box_init[1] = Interval(-1, 1);

    Function f_sep_outer(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain dom_inner(&paving, FULL_DOOR);

    Function f_sep_inner(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f_outer(x1, x2, Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+2.0));
    ibex::Function f_inner(x1, x2, -Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+2.0));
    Dynamics_Function dyn_outer(&f_outer, FWD);
    Dynamics_Function dyn_inner(&f_inner, BWD);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer);
    Maze maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.init();
    maze_inner.init();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    Vibes_Graph v_graph("SmartSubPaving", &paving, &maze_outer, &maze_inner);
//    Vibes_Graph v_graph("SmartSubPaving", &paving, &maze_inner);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

    v_graph.drawCircle(0.0, 0.0, 1, "black[red]");

    IntervalVector position_info(2);
    position_info[0] = Interval(-1.2, -1.16);
    position_info[1] = Interval(3.62);
//    v_graph.get_room_info(&maze_inner, position_info);
//    v_graph.show_room_info(&maze_inner, position_info);


    vibes::endDrawing();

}
