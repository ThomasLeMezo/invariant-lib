#include "ibex/ibex_SepFwdBwd.h"
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
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = Interval(-3,3);
    space[1] = Interval(-3,3.1);

    // ****** Domain ******* //
    Graph graph(space);
    invariant::Domain dom(&graph);

    Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(0.5, 2));
    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                    (8.0/25.0*pow(x1,5)-4.0/3.0*pow(x1,3)+4.0/5.0*x1)));
    Dynamics_Function dyn(&f);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn, MAZE_FWD_BWD, MAZE_CONTRACTOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        graph.bisect();
        cout << i << " - " << maze.contract() << " - ";
        cout << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph("graph", &graph, &maze);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

    IntervalVector position_info(2);
    position_info[0] = Interval(1.9);
    position_info[1] = Interval(0.3, 0.32);
//    v_graph.get_room_info(&maze, position_info);
    v_graph.show_room_info(&maze, position_info);

    vibes::endDrawing();

}
