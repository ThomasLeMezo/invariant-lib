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
    space[0] = Interval(-1.0, 13.0);
    space[1] = Interval(-16, 16);

    Graph graph(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&graph);

    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain dom_inner(&graph);

    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    // ****** Dynamics Outer ******* //
    ibex::Function f_outer(x1, x2, Return(x2,
                                     (-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + Interval(-0.5, +0.5))));

    Dynamics_Function dyn_outer(&f_outer);

    // ****** Dynamics Inner ******* //
    ibex::Function f_inner1(x1, x2, Return(-x2,
                                     -(-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + Interval(-0.5))));
    ibex::Function f_inner2(x1, x2, Return(-x2,
                                     -(-9.81*sin( (-1.1/1.2*sin(x1)-1.2*sin(1.1*x1))/2.0 ) -0.7*x2 + Interval(+0.5))));
    vector<Function *> f_list_inner;
    f_list_inner.push_back(&f_inner1);
    f_list_inner.push_back(&f_inner2);
    Dynamics_Function dyn_inner(f_list_inner);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer, MAZE_BWD, MAZE_CONTRACTOR);
    Maze maze_inner(&dom_inner, &dyn_inner, MAZE_FWD, MAZE_PROPAGATOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_inner.contract();
    for(int i=0; i<15; i++){
        graph.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << graph.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    vibes::beginDrawing();
//    Vibes_Graph v_graph_outer("graph_outer", &graph, &maze_outer);
//    v_graph_outer.setProperties(0, 0, 1024, 1024);
//    v_graph_outer.show();

//    Vibes_Graph v_graph_inner("graph_inner", &graph, &maze_inner);
//    v_graph_inner.setProperties(0, 0, 1024, 1024);
//    v_graph_inner.show();

    Vibes_Graph v_graph("graph_inner", &graph, &maze_outer, &maze_inner);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

    vibes::endDrawing();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-1.7);
//    position_info[1] = Interval(1);
//    v_graph_outer.get_room_info(&maze, position_info);

}