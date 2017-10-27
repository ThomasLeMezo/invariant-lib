#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
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
    space[0] = Interval(-10,10);
    space[1] = Interval(-10,10);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function h(x1, x2, 2*exp(-1.0*(pow(x1+2, 2)+pow(x2+2,2))/10.0) + 2*exp(-1.0*(pow(x1-2,2)+pow(x2-2,2))/10.0) - 10.0);
    ibex::Function hdiff(h, Function::DIFF);
    ibex::Function psi(x1, x2, tanh(h(x1, x2))-9);
    ibex::Function phi_bar(x1, x2, psi(x1, x2)+atan2(hdiff(x1, x2)[1], hdiff(x1, x2)[0])+Interval::PI/2.0);
//    ibex::Function f(x1, x2, Return(cos(phi_bar(x1, x2)), sin(phi_bar(x1, x2))));
    ibex::Function f(x1, x2, Return(cos(phi_bar(x1, x2)), sin(phi_bar(x1, x2))));

//    ibex::Function f(x1, x2, Return(x2,
//                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn(&f, FWD_BWD);

//    IntervalVector test(2);
//    test[0] = Interval(0, 0.5);
//    test[1] = Interval(0, 0.5);
//    cout << f.eval(test) << endl;

    // ******* Maze ********* //
    Maze maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    maze.init();
    for(int i=0; i<20; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - ";
        cout << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph("SmartSubPaving", &paving, &maze);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(0);
//    position_info[1] = Interval(1);
//    v_graph.get_room_info(&maze, position_info);

    IntervalVector position_info(2);
    position_info[0] = Interval(-2.5);
    position_info[1] = Interval(2.9);
//    v_graph.get_room_info(&maze, position_info);
//    v_graph.show_room_info(&maze, position_info);

//    position_info[0] = Interval(0);
//    position_info[1] = Interval(-1);
//    v_graph.get_room_info(&maze, position_info);
    vibes::endDrawing();

}
