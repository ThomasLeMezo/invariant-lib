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
    ibex::Variable x(2);

    IntervalVector space(2);
    space[0] = Interval(-4,4);
    space[1] = Interval(-4,4);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep_outer(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_outer.set_sep_output(&s_outer);

    invariant::Domain dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep_inner(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
    ibex::Function f_outer(x, Return(x[1],
                                    (1.0*(1.0-pow(x[0], 2))*x[1]-x[0])));
    Dynamics_Function dyn_outer(&f_outer, FWD);

    ibex::Function f_inner(x, -Return(x[1],
                           (1.0*(1.0-pow(x[0], 2))*x[1]-x[0])));
    Dynamics_Function dyn_inner(&f_inner, BWD);

    // ******* Maze ********* //
    Maze maze_outer(&dom_outer, &dyn_outer);
    Maze maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.init();
    maze_inner.init();
    for(int i=0; i<13; i++){
        paving.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph("SmartSubPaving", &maze_outer, &maze_inner);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

//    Vibes_Graph v_graph_inner("graph_inner",&maze_inner, Vibes_Graph::VIBES_GRAPH_INNER);
//    v_graph_inner.setProperties(0, 0, 512, 512);
//    v_graph_inner.show();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-0.4);
//    position_info[1] = Interval(1.34);
//    v_graph.get_room_info(&maze_inner, position_info);

//    position_info[0] = Interval(-0.34);
//    position_info[1] = Interval(1.34);
//    v_graph.get_room_info(&maze_inner, position_info);

    v_graph.drawCircle(x1_c, x2_c, r, "black[red]");

    vibes::endDrawing();

}
