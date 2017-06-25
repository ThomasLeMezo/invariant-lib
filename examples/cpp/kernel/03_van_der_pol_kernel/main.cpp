#include "graph.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibes_graph.h"

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
    space[0] = Interval(-6,6);
    space[1] = Interval(-6,6);

    Graph graph(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&graph);

//    Function f_sep_outer(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(1.0, 2));
//    SepFwdBwd s_outer(f_sep_outer, GEQ); // LT, LEQ, EQ, GEQ, GT
//    dom_outer.set_sep(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain dom_inner(&graph);

//    Function f_sep_inner(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(1.0, 2));
//    SepFwdBwd s_inner(f_sep_inner, LEQ); // LT, LEQ, EQ, GEQ, GT
//    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer ******* //
    ibex::Function f_outer(x1, x2, Return(-x2,
                                    -(1.0*(1.0-pow(x1, 2))*x2-x1)+Interval(-0.3, 0.3)));
    Dynamics_Function dyn_outer(&f_outer);

    // ****** Dynamics Inner ******* //
    ibex::Function f_inner(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+Interval(-0.3, 0.3)));
    Dynamics_Function dyn_inner(&f_inner);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer, MAZE_BWD, MAZE_CONTRACTOR);
    Maze maze_inner(&dom_inner, &dyn_inner, MAZE_FWD, MAZE_PROPAGATOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_inner.contract();
    for(int i=0; i<17; i++){
        graph.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << graph.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph_outer("graph_outer", &graph, &maze_outer);
    v_graph_outer.setProperties(0, 0, 1024, 1024);
    v_graph_outer.show();

    Vibes_Graph v_graph_inner("graph_inner", &graph, &maze_inner);
    v_graph_inner.setProperties(0, 0, 1024, 1024);
    v_graph_inner.show();

    vibes::endDrawing();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-1.7);
//    position_info[1] = Interval(1);
//    v_graph.get_room_info(&maze, position_info);

}
