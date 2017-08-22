#include "ibex/ibex_SepFwdBwd.h"
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
    space[0] = Interval(-1, 1);
    space[1] = Interval(-1, 1);

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
    ibex::Function f_outer(x1, x2, Return(pow(x2, 2),
                                          Interval(-1, 1)));

    Dynamics_Function dyn_outer(&f_outer);

    // ****** Dynamics Inner ******* //
    ibex::Function f_inner1(x1, x2, Return(pow(x2, 2),
                                          Interval(-1)));
    ibex::Function f_inner2(x1, x2, Return(pow(x2, 2),
                                          Interval(1)));

    vector<Function *> f_list_inner;
    f_list_inner.push_back(&f_inner1);
    f_list_inner.push_back(&f_inner2);
    Dynamics_Function dyn_inner(f_list_inner);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer, MAZE_BWD, MAZE_CONTRACTOR);
    Maze maze_inner(&dom_inner, &dyn_inner, MAZE_FWD, MAZE_PROPAGATOR);

    // ******* Algorithm ********* //
    vibes::beginDrawing();
    double time_start = omp_get_wtime();
    maze_inner.contract();
    for(int i=0; i<15; i++){
        graph.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << graph.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vibes_Graph v_graph("graph_inner", &graph, &maze_outer, &maze_inner);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

    vibes::endDrawing();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-1.7);
//    position_info[1] = Interval(1);
//    v_graph_outer.get_room_info(&maze, position_info);

}
