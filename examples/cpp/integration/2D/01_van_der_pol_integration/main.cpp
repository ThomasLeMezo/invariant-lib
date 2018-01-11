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
    space[0] = ibex::Interval(-6,6);
    space[1] = ibex::Interval(-6,6);

    // ****** Domain *******
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 3.0;
    x2_c = 2.0;
    r = 0.3;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LT); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    vector<Function *> f_list;
    f_list.push_back(&f);
    Dynamics_Function dyn(f_list, FWD);

    // ******* Maze *********
    invariant::Maze<> maze(&dom, &dyn);

    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("SmartSubPaving", &maze);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();
    v_maze.drawCircle(3, 2, 0.3, "red[]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(3.1);
//    position_info[1] = ibex::Interval(1.8);
//    v_maze.show_room_info(&maze, position_info);

//    IntervalVector pave_in(2);
//    pave_in[0] = ibex::Interval(2.67188, 2.71875);
//    pave_in[1] = ibex::Interval(2.0625, 2.15625);
//    IntervalVector pave_out(pave_in);
//    s.separate(pave_in, pave_out);
//    cout << "pave_in = " << pave_in << " pave_out = " << pave_out << endl;
//    vibes::drawBox(pave_out, "black[]");

}
