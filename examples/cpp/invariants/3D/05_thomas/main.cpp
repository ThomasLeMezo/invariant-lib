#include "ibex/ibex_SepFwdBwd.h"
#include "ibex/ibex_SepInter.h"
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
    space[0] = Interval(-10, 10);
    space[1] = Interval(-10, 10);
    space[2] = Interval(-10, 10);

    // ****** Domain ******* //
    Graph graph(space);
    invariant::Domain dom(&graph);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double r = 0.1;
    Array<Sep> array_sep;
    Function f_sep1(x, y, z, pow(x, 2)+pow(y, 2)+pow(z, 2)-pow(r, 2));
    SepFwdBwd s1(f_sep1, GEQ); // LT, LEQ, EQ, GEQ, GT
    array_sep.add(s1);

//    Function f_sep2(x1, x2, x3, pow(x1-pt_xy, 2)+pow(x2-pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
//    SepFwdBwd s2(f_sep2, GEQ); // LT, LEQ, EQ, GEQ, GT
//    array_sep.add(s2);

//    Function f_sep3(x1, x2, x3, pow(x1+pt_xy, 2)+pow(x2+pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
//    SepFwdBwd s3(f_sep3, GEQ); // LT, LEQ, EQ, GEQ, GT
//    array_sep.add(s3);

    SepInter sep_total(array_sep);
    dom.set_sep(&sep_total);

    // ****** Dynamics ******* //
//    Interval b = Interval(0.32899);
//    Interval b = Interval(0.208186);
    Interval b = Interval(0.1998);

    ibex::Function f(x, y, z, Return(sin(y)-b*x,
                                     sin(z)-b*y,
                                     sin(x)-b*z));
    Dynamics_Function dyn(&f);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn, MAZE_FWD_BWD, MAZE_DOOR);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<20; i++){
        cout << "-----" << i << "-----" << endl;
        graph.bisect();
        cout << "nb contractions = " << maze.contract() << " - ";
        cout << "graph size = " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vtk_Graph vtk_graph("thomas", &graph, true);
//    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze);

    //    IntervalVector position_info(2);
    //    position_info[0] = Interval(-1.7);
    //    position_info[1] = Interval(1);
    //    v_graph.get_room_info(&maze, position_info);

}
