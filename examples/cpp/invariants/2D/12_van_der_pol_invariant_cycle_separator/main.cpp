#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "sepmaze.h"

#include "ibex_SepFwdBwd.h"

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
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_DOOR);

    Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(1.0, 2));
    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
    DynamicsFunction dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<10; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    IntervalVector box_in(2), box_out(2);
    box_in[0] = ibex::Interval(2.1, 2.4);
    box_in[1] = ibex::Interval(-0.370, -0.05);
//    box_in[0] = ibex::Interval(1.8, 2.8);
//    box_in[1] = ibex::Interval(-2, -0.5);
    box_out = box_in;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();

    SepMaze<> sepMaze(&maze);

    cout << "box = " << box_in << endl;
//    vibes::drawBox(box_in, "red[]");
    sepMaze.separate(box_in, box_out);
    vibes::drawBox(box_in, "red[]");
    vibes::drawBox(box_out, "green[]");

    cout << "in = " << box_in << endl;
    cout << "out = " << box_out << endl;

    vibes::endDrawing();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.3);
//    position_info[1] = ibex::Interval(-0.95);
//    v_maze.get_room_info(&maze, position_info);

}
