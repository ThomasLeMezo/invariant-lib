#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibes_graph.h"

#include <iostream>
#include "vibes/vibes.h"
#include "graphiz_graph.h"
#include <omp.h>
#include "vtk_graph.h"

#include "ibex/ibex_SepFwdBwd.h"
#include "ibex/ibex_SepInter.h"

#include <string>

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

    Interval rho = Interval(28.0);
    Interval sigma = Interval(10.0);
    Interval beta = Interval(8.0/3.0);

    // ****** Domain ******* //
    SmartSubPaving paving(space);
    invariant::Domain dom(&paving, FULL_DOOR);
    Interval pt_xy = sqrt(beta*(rho-1.0));
    Interval pt_z = rho-1.0;
    double r = 3.0;

    // Remove zeros
    Function f_sep1(x1, x2, x3, pow(x1, 2)+pow(x2, 2)+pow(x3, 2)-pow(r, 2));
    SepFwdBwd s1(f_sep1, GEQ); // LT, LEQ, EQ, GEQ, GT
    Function f_sep2(x1, x2, x3, pow(x1-pt_xy, 2)+pow(x2-pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
    SepFwdBwd s2(f_sep2, GEQ); // LT, LEQ, EQ, GEQ, GT
    Function f_sep3(x1, x2, x3, pow(x1+pt_xy, 2)+pow(x2+pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
    SepFwdBwd s3(f_sep3, GEQ); // LT, LEQ, EQ, GEQ, GT

    Array<Sep> array_sep;
    array_sep.add(s1);
    array_sep.add(s2);
    array_sep.add(s3);
    SepInter sep_total(array_sep);
    dom.set_sep(&sep_total);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //

    //    Interval rho = Interval(13.0);
    //    Interval sigma = Interval(10.0);
    //    Interval beta = Interval(8.0/3.0);

    ibex::Function f(x1, x2, x3, Return(sigma * (x2 - x1),
                                        x1*(rho - x3) - x2,
                                        x1*x2 - beta * x3));
    Dynamics_Function dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    Maze maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze.init();
    for(int i=0; i<20; i++){
        cout << "-----" << i << "-----" << endl;
        paving.bisect();
        cout << "nb contractions = " << maze.contract() << " - " << "paving size = " << paving.size() << endl;
        //        vtk_graph.show_maze(&maze, std::to_string(i));
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;
    cout << paving << endl;

    Vtk_Graph vtk_graph("lorenz", &paving, true);
//    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze);

//    IntervalVector position_info(3);
//    position_info[0] = Interval(0.5);
//    position_info[1] = Interval(0.5);
//    position_info[2] = Interval(0.5);
//    vtk_graph.show_room_info(&maze, position_info);
    //        vector<Pave*> pave_list;
    //        paving.get_room_info(&maze, position_info, pave_list);

}
