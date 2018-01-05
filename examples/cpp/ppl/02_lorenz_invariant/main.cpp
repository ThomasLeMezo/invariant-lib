#include "ibex_SepFwdBwd.h"
#include "ibex_SepInter.h"
#include "ibex_SepUnion.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include <cstring>
#include <omp.h>

#include "vtkmazeppl.h"

#include "ppl.hh"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2, x3;

    IntervalVector space(3);
    space[0] = ibex::Interval(-30,30);
    space[1] = ibex::Interval(-20,20);
    space[2] = ibex::Interval(0,50);

    ibex::Interval rho = ibex::Interval(28.0);
    ibex::Interval sigma = ibex::Interval(10.0);
    ibex::Interval beta = ibex::Interval(8.0/3.0);

    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);
    invariant::DomainPPL dom(&paving, FULL_DOOR);
    ibex::Interval pt_xy = sqrt(beta*(rho-1.0));
    ibex::Interval pt_z = rho-1.0;
    double r = 3.0;
    double r2 = 5.0;

    // Remove zeros
    Function f_sep2(x1, x2, x3, pow(x1-pt_xy, 2)+pow(x2-pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
    SepFwdBwd s2(f_sep2, GEQ); // LT, LEQ, EQ, GEQ, GT
    Function f_sep3(x1, x2, x3, pow(x1+pt_xy, 2)+pow(x2+pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
    SepFwdBwd s3(f_sep3, GEQ); // LT, LEQ, EQ, GEQ, GT

    Function f_sep1(x1, x2, x3, pow(x1, 2)+pow(x2, 2)+pow(x3, 2)-pow(r2, 2));
    SepFwdBwd s1(f_sep1, GEQ); // LT, LEQ, EQ, GEQ, GT

    Array<Sep> array_sep;
    array_sep.add(s1);
    array_sep.add(s2);
    array_sep.add(s3);
    SepInter sep_total(array_sep);
    dom.set_sep(&sep_total);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, x3, Return(sigma * (x2 - x1),
                                        x1*(rho - x3) - x2,
                                        x1*x2 - beta * x3));
    Dynamics_Function dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    MazePPL maze(&dom, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    VtkMazePPL vtkMazePPL("LorenzPPL");

//    omp_set_num_threads(1);

    maze.set_limit_contraction_door(true);
    maze.set_widening_limit(5);

    for(int i=0; i<15; i++){
        std::time_t t_now = std::time(nullptr);
        cout << i << " - " << std::ctime(&t_now);
        paving.bisect();
        maze.contract(paving.size_active()*5);
        vtkMazePPL.show_maze(&maze);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;
}
