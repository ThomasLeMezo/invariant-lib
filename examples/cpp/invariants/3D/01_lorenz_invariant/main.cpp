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
    ibex::Variable x1, x2, x3;

    IntervalVector space(3);
    space[0] = Interval(-30,30);
    space[1] = Interval(-20,20);
    space[2] = Interval(0,50);

//    space[0] = Interval(-3,13);
//    space[1] = Interval(-1,15);
//    space[2] = Interval(-1, 21);

    // ****** Domain ******* //
    Graph graph(space);
    invariant::Domain dom(&graph);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    Interval rho = Interval(28.0);
    Interval sigma = Interval(10.0);
    Interval beta = Interval(8.0/3.0);

//    Interval rho = Interval(13.0);
//    Interval sigma = Interval(10.0);
//    Interval beta = Interval(8.0/3.0);

    ibex::Function f(x1, x2, x3, Return(sigma * (x2 - x1),
                                        x1*(rho - x3) - x2,
                                        x1*x2 - beta * x3));
    Dynamics_Function dyn(&f);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn, MAZE_FWD_BWD, MAZE_CONTRACTOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<18; i++){
        cout << "-----" << i << "-----" << endl;
        graph.bisect();
        cout << "nb contractions = " << maze.contract() << " - ";
        cout << "graph size = " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vtk_Graph vtk_graph("lorenz", &graph, true);
//    vtk_graph.show_graph();
//    vtk_graph.show_maze(&maze);

//    vector<Pave*> pave_list;
    IntervalVector position_info(3);
//    position_info[0] = Interval(-30.0);
//    position_info[1] = Interval(-2.5);
//    position_info[2] = Interval(28.0);
//    graph.get_room_info(&maze, position_info, pave_list);

    position_info[0] = Interval(29.5);
    position_info[1] = Interval(3.0);
    position_info[2] = Interval(28.0);
//    graph.get_room_info(&maze, position_info, pave_list);
    vtk_graph.show_room_info(&maze, position_info);
}
