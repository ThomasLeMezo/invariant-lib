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

    // ****** Domain *******
    Graph graph(space);
    invariant::Domain dom(&graph);

    double x1_c, x2_c, r;
    x1_c = 3.0;
    x2_c = 2.0;
    r = 1.0;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn(&f);

    // ******* Maze *********
    Maze maze(&dom, &dyn, MAZE_FWD, MAZE_PROPAGATOR);

    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        graph.bisect();
        cout << i << " - " << maze.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vibes_Graph v_graph("graph", &graph, &maze);
    v_graph.setProperties(0, 0, 512, 512);
    v_graph.show();

    IntervalVector position_info(2);
    position_info[0] = Interval(-2);
    position_info[1] = Interval(4);
    v_graph.get_room_info(&maze, position_info);

}
