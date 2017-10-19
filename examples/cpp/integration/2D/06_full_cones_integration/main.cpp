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
    space[0] = Interval(-1, 1);
    space[1] = Interval(-1, 1);

    Graph graph(space);

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&graph, FULL_WALL);

    double x1_0, x2_0, r;
    x1_0 = 0.0;
    x2_0 = 0.0;
    r = 0.1;

    Function f_sep_outer(x1, x2, pow(x1-x1_0, 2)+pow(x2-x2_0, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LT); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x1, x2, Return(Interval(-1, 1),
                                    Interval(-1, 1)));

    Dynamics_Function dyn_outer(&f, FWD);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.contract();
    for(int i=0; i<15; i++){
        graph.bisect();
        cout << i << " outer - " << maze_outer.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph("graph", &graph, &maze_outer);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

    //    IntervalVector position_info(2);
    //    position_info[0] = Interval(-2);
    //    position_info[1] = Interval(-2);
    //    v_graph_outer.get_room_info(&maze_outer, position_info);

    vibes::endDrawing();

}
