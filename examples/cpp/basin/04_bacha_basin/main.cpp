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

// Note : a integration approach for the outer approximation is more suitable
// In face it avoid any issue with stability point that may let impossible any
// convergence toward the boundary of the set

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x(2);

    IntervalVector space(2);
    space[0] = Interval(-6,6);
    space[1] = Interval(-6,6);

    // ****** Domain ******* //
    Graph graph(space);
    invariant::Domain dom_outer(&graph);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep_outer(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_outer.set_sep_output(&s_outer);

    invariant::Domain dom_inner(&graph);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

//    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep_inner(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
    ibex::Function f_outer(x, Return(-(x[1]),
                           -(-0.5*x[1]-sin(x[0]+0.412)+sin(0.412))));
    ibex::Function f_inner(x, Return((x[1]),
                           (-0.5*x[1]-sin(x[0]+0.412)+sin(0.412))));
    Dynamics_Function dyn_outer(&f_outer);
    Dynamics_Function dyn_inner(&f_inner);

    // ******* Maze ********* //
    Maze maze_outer(&dom_outer, &dyn_outer, MAZE_FWD, MAZE_WALL);
    Maze maze_inner(&dom_inner, &dyn_inner, MAZE_BWD, MAZE_DOOR);

    // ******* Algorithm ********* //
//    vibes::beginDrawing();
    double time_start = omp_get_wtime();
    maze_inner.init();
    maze_outer.init();
    for(int i=0; i<15; i++){
        graph.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << graph.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vibes_Graph v_graph("graph", &graph, &maze_outer, &maze_inner);
    v_graph.setProperties(0, 0, 512, 512);
    v_graph.show();
    v_graph.drawCircle(x1_c, x2_c, r, "red[]");

//    Vibes_Graph v_graph_inner("graph_inner", &graph,&maze_inner, Vibes_Graph::VIBES_GRAPH_INNER);
//    v_graph_inner.setProperties(0, 0, 512, 512);
//    v_graph_inner.show();

//    Vibes_Graph v_graph_outer("graph_outer", &graph,&maze_outer, Vibes_Graph::VIBES_GRAPH_OUTER);
//    v_graph_outer.setProperties(0, 0, 512, 512);
//    v_graph_outer.show();

//    Vibes_Graph v_graph_info("graph_info", &graph, &maze_inner);
//    v_graph_info.setProperties(0, 0, 512, 512);
//    v_graph_info.show();
//    IntervalVector position_info(2);
//    position_info[0] = Interval(-1);
//    position_info[1] = Interval(2, 4);
//    v_graph_info.setProperties(0, 0, 300, 300);
//    v_graph_info.show_room_info(&maze_inner, position_info);

    vibes::endDrawing();

}
