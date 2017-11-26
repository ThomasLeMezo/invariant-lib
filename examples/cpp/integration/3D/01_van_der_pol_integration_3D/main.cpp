#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include <iostream>
#include <cstring>
#include <omp.h>

#include "vtkMaze3D.h"

#include "ibex/ibex_SepFwdBwd.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2, x3;

    IntervalVector space(3);
    space[0] = ibex::Interval(-6,6);
    space[1] = ibex::Interval(-6,6);
    space[2] = ibex::Interval(0,20);

    // ****** Domain *******
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, invariant::Domain<>::FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double x1_c, x2_c, x3_c, r;
    x1_c = 3.0;
    x2_c = 2.0;
    x3_c = 1.0;
    r = 0.3;

    Function f_sep(x1, x2, x3, pow(x1-x1_c, 2)+pow(x2-x2_c, 2) + pow(x3-x3_c, 2) - pow(r, 2));
    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, x3, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1), pow(x2, 2)+ pow((1.0*(1.0-pow(x1, 2))*x2-x1), 2)));
    vector<Function *> f_list;
    f_list.push_back(&f);
    Dynamics_Function dyn(f_list, Dynamics::FWD);

    // ******* Maze *********
    invariant::Maze<> maze(&dom, &dyn);

    double time_start = omp_get_wtime();
    maze.contract(); // To set first pave to be in
    for(int i=0; i<20; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VtkMaze3D vtkMaze3D("vdp_3D");
    vtkMaze3D.show_graph(&paving);
    vtkMaze3D.show_maze(&maze);

}
