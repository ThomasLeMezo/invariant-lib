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
#include "vtk_graph.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x, y, z;

    IntervalVector space(3);
    space[0] = Interval(-4, 2);
    space[1] = Interval(-2, 2);
    space[2] = Interval(-2, 3);

    // ****** Domain ******* //
    Graph graph(space);
    invariant::Domain dom(&graph);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
//    Interval gamma = Interval(0.87);
//    Interval alpha = Interval(1.1);

    Interval gamma = Interval(0.1);
    Interval alpha = Interval(0.14);


    ibex::Function f(x, y, z, Return(y*(z-1+pow(x,2))+gamma*x,
                                     x*(2*z+1-pow(x,2))+gamma*y,
                                     -2*z*(alpha+x*y)));
    Dynamics_Function dyn(&f);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn, MAZE_FWD_BWD, MAZE_CONTRACTOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        cout << "-----" << i << "-----" << endl;
        graph.bisect();
        cout << "nb contractions = " << maze.contract() << " - ";
        cout << "graph size = " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vtk_Graph vtk_graph("rabinovich-fabrikant", &graph, true);
    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze);

    //    IntervalVector position_info(2);
    //    position_info[0] = Interval(-1.7);
    //    position_info[1] = Interval(1);
    //    v_graph.get_room_info(&maze, position_info);

}
