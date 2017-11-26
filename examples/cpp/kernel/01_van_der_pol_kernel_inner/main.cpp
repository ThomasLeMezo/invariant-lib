#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

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
    space[0] = ibex::Interval(-6,6);
    space[1] = ibex::Interval(-6,6);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, invariant::Domain<>::FULL_WALL);

    Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(1.0, 2));
    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT
//    dom.set_sep(&s);

    dom.set_border_path_in(true);
    dom.set_border_path_out(true);

    // ****** Dynamics ******* //
    ibex::Function f1(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+ibex::Interval(-0.3)));
    ibex::Function f2(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+ibex::Interval(0.3)));

//    ibex::Function f1(x1, x2, Return(Interval(1.0),
//                                    ibex::Interval(0.5)));
//    ibex::Function f2(x1, x2, Return(Interval(1.0),
//                                     ibex::Interval(-0.5)));

    vector<Function *> f_list;
    f_list.push_back(&f1);
    f_list.push_back(&f2);
    Dynamics_Function dyn(f_list, Dynamics::FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze.contract(); // To init the first room in case of Propagator
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze, VibesMaze::VIBES_MAZE_INNER);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    vibes::endDrawing();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(1);
//    position_info[1] = ibex::Interval(1);
//    v_maze.get_room_info(&maze, position_info);
}
