#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(0,3);
    space[1] = ibex::Interval(-10,10);

    // ****** Domain *******
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 2.0;
    x2_c = 0.0;
    r = 0.1;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LT); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(x2,-9.81+0*x2));
    Dynamics_Function dyn(&f, FWD);

    // Hybrid
    ibex::Function f_sep_guard(x1, x2, x1);
    ibex::SepFwdBwd sep_guard(f_sep_guard, GEQ);

    ibex::Function f_reset_pos(x1, x2, Return(x1, -x2));
    ibex::Function f_reset_neg(x1, x2, Return(x1, -x2));
    dyn.add_hybrid_condition(&sep_guard, &f_reset_pos, &f_reset_neg);

    // ******* Maze *********
    invariant::Maze<> maze(&dom, &dyn);

    omp_set_num_threads(1);
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("Hybrid Ball", &maze);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();
    v_maze.drawCircle(x1_c, x2_c, r, "red[]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(3.1);
//    position_info[1] = ibex::Interval(1.8);
//    v_maze.show_room_info(&maze, position_info);


}
