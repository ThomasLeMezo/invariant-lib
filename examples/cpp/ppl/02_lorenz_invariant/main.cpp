#include "ibex/ibex_SepFwdBwd.h"
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
    SmartSubPavingPPL paving(space);
    DomainPPL dom(&paving, FULL_DOOR);

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

    omp_set_num_threads(1);
    for(int i=0; i<10; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - ";
        cout << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VtkMazePPL vtkMazePPL("LorenzPPL");
    vtkMazePPL.show_maze(&maze);
}
