#include "smartSubPaving.h"
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
    space[0] = Interval(-6,6);
    space[1] = Interval(-6,6);

    // ****** Domain *******
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_WALL);

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
    ibex::Function f(x1, x2, Return(Interval(-0.01, 0.01), Interval(1)));
    Dynamics_Function dyn(&f, FWD);

    // ******* Maze *********
    Maze maze(&dom, &dyn);

    double time_start = omp_get_wtime();
    for(int i=0; i<14; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    Vibes_Graph v_graph("paving", &paving, &maze);
    v_graph.setProperties(0, 0, 512, 512);
    v_graph.show();

    IntervalVector position_info(2);
    position_info[0] = Interval(-0.09);
    position_info[1] = Interval(-2.976);
    v_graph.get_room_info(&maze, position_info);

}
