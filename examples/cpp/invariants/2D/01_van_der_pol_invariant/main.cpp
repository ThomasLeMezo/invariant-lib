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
    space[0] = Interval(-3,3);
    space[1] = Interval(-3,3.1);

    // ****** Domain ******* //
    Graph graph(space);
    invariant::Domain dom(&graph);

    dom.set_border_path_in(false);
    dom.set_border_path_out(true);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn(&f);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn, MAZE_FWD, MAZE_CONTRACTOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<6; i++){
        graph.bisect();
        cout << i << " - " << maze.contract() << " - ";
        cout << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph("graph", &graph, &maze);
    v_graph.setProperties(0, 0, 512, 512);
    v_graph.show();

    IntervalVector position_info(2);
//    position_info[0] = Interval(0);
//    position_info[1] = Interval(1);
//    v_graph.get_room_info(&maze, position_info);

    position_info[0] = Interval(-2);
    position_info[1] = Interval(1);
//    v_graph.get_room_info(&maze, position_info);
//    v_graph.show_room_info(&maze, position_info);

//    position_info[0] = Interval(0);
//    position_info[1] = Interval(-1);
//    v_graph.get_room_info(&maze, position_info);
    vibes::endDrawing();

}
