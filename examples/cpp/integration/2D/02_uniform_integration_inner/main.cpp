#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

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
    space[0] = ibex::Interval(-6,6);
    space[1] = ibex::Interval(-6,6);

    // ****** Domain *******
    invariant::SmartSubPavingEXP paving(space);
    invariant::DomainEXP dom(&paving, FULL_WALL);

    double x1_c, x2_c, r;
    x1_c = -2.0;
    x2_c = -4.0;
    r = 0.3;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    dom.set_border_path_in(true);
    dom.set_border_path_out(true);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(ibex::Interval(-0.01, 0.01), ibex::Interval(1)));
    DynamicsFunction dyn(&f, FWD);

    // ******* Maze *********
    invariant::MazeEXP maze(&dom, &dyn);

    double time_start = omp_get_wtime();
    for(int i=0; i<14; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMazeEXP v_maze("SmartSubPavingEXP", &maze);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(-0.09);
    position_info[1] = ibex::Interval(-2.976);
    v_maze.get_room_info(&maze, position_info);

}
