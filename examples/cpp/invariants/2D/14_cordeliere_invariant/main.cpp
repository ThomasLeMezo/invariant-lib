#include "ibex/ibex_SepFwdBwd.h"
#include "ibex/ibex_SepInter.h"
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
    space[0] = Interval(-10, 10);
    space[1] = Interval(-10, 10);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    Function f_sep(x1, x2, sqr(x1-1.78129)+sqr(x2-1.78129)-0.5);
    Function f_sep2(x1, x2, sqr(x1+1.78129)+sqr(x2+1.78129)-0.5);
    Function f_sep3(x1, x2, sqr(x1)+sqr(x2)-0.5);
    SepFwdBwd sep(f_sep, GEQ);
    SepFwdBwd sep2(f_sep2, GEQ);
    SepFwdBwd sep3(f_sep3, GEQ);
    SepInter sep_i(sep, sep2, sep3);
    dom.set_sep(&sep_i);

    // ****** Dynamics ******* //
    double h0 = -9;

    ibex::Function h(x1, x2, 2.0*exp((sqr(x1+2)+sqr(x2+2))/-10.0) + 2.0*exp((sqr(x1-2)+sqr(x2-2))/-10.0) - 10.0);

    ibex::Function hdiff_x1(x1, x2, -0.4*(x1-2)*exp(-0.1*(sqr(x1-2)+sqr(x2-2)))-0.4*(x1+2)*exp(-0.1*(sqr(x1+2)+sqr(x2+2))));
    ibex::Function hdiff_x2(x1, x2, -0.4*(x2-2)*exp(-0.1*(sqr(x1-2)+sqr(x2-2)))-0.4*(x2+2)*exp(-0.1*(sqr(x1+2)+sqr(x2+2))));

    ibex::Function psi(x1, x2, tanh(h(x1, x2)-h0)+Interval::PI/2.0);

    ibex::Function f(x1, x2, Return((hdiff_x1(x1, x2)*cos(psi(x1, x2))-hdiff_x2(x1, x2)*sin(psi(x1, x2)))/sqrt(sqr(hdiff_x2(x1, x2))+sqr(hdiff_x1(x1, x2))),
                                    (hdiff_x2(x1, x2)*cos(psi(x1, x2))+hdiff_x1(x1, x2)*sin(psi(x1, x2)))/sqrt(sqr(hdiff_x2(x1, x2))+sqr(hdiff_x1(x1, x2)))));

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
    VibesMaze v_maze("SmartSubPaving", &maze);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    v_maze.drawCircle(0, 0, 0.5, "black[red]");
    v_maze.drawCircle(1.78129, 1.78129, 0.5, "black[red]");
    v_maze.drawCircle(-1.78129, -1.78129, 0.5, "black[red]");

    IntervalVector position_info(2);
    position_info[0] = Interval(2.22);
    position_info[1] = Interval(1.97);
//    v_maze.get_room_info(&maze, position_info);
//    v_maze.show_room_info(&maze, position_info);

//    position_info[0] = Interval(0);
//    position_info[1] = Interval(-1);
//    v_maze.get_room_info(&maze, position_info);
    vibes::endDrawing();

//    IntervalVector test(2);
//    test[0] = Interval(2.1875, 2.265625);
//    test[1] = Interval(1.875, 2.03125);
//    cout << "h = " << h.eval_vector(test) << endl;
//    cout << "dh_x1 = " << hdiff_x1.eval_vector(test) << endl;
//    cout << "dh_x2 = " << hdiff_x2.eval_vector(test) << endl;
//    cout << "psi = " << psi.eval_vector(test) << endl;
//    cout << "phi_bar = " << phi_bar.eval_vector(test) << endl;
//    cout << "f = " << f.eval_vector(test) << endl;

//    ibex::Function t(x1, x2, Return(cos(x1), sin(x2)));
//    IntervalVector iv(2);
//    iv[0] = Interval(-11.5708, -5.28761);
//    iv[1] = Interval(-11.5708, -5.28761);
//    cout << t.eval_vector(iv) << endl;

}
