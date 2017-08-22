#include "graph.h"
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
    ibex::Variable x, t;

    IntervalVector space(2);
    space[0] = Interval(-0.1,8);
    space[1] = Interval(-3,3);

    Graph graph(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&graph);

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
    invariant::Domain dom_inner(&graph);

    SepFwdBwd s_inner(f_sep_outer, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    Interval a = Interval(10.0);
    ibex::Function f(t, x, Return(Interval(1.0),
                                    -a*x+(-1+a)*sin(t)+(1+a)*cos(t)));

    Dynamics_Function dyn_outer(&f);
    Dynamics_Function dyn_inner(&f);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer, MAZE_FWD, MAZE_PROPAGATOR);
    Maze maze_inner(&dom_inner, &dyn_inner, MAZE_FWD, MAZE_CONTRACTOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.contract();
    for(int i=0; i<18; i++){
        graph.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << graph.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph("graph", &graph, &maze_outer, &maze_inner);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

    //    IntervalVector position_info(2);
    //    position_info[0] = Interval(-2);
    //    position_info[1] = Interval(-2);
    //    v_graph_outer.get_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
