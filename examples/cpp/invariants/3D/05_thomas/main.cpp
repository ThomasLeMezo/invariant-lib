#include "ibex_SepFwdBwd.h"
#include "ibex_SepInter.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>
//#include "vtkMaze3D.h"
#include "vtkmazeppl.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x, y, z;

    IntervalVector space(3);
//    space[0] = ibex::Interval(-10, 10);
//    space[1] = ibex::Interval(-10, 10);
//    space[2] = ibex::Interval(-10, 10);

    space[0] = ibex::Interval(-4, 4);
    space[1] = ibex::Interval(-4, 4);
    space[2] = ibex::Interval(-4, 4);

    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);
    invariant::DomainPPL dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double r = 0.1;
    Array<Sep> array_sep;
    Function f_sep1(x, y, z, pow(x, 2)+pow(y, 2)+pow(z, 2)-pow(r, 2));
    SepFwdBwd s1(f_sep1, GEQ); // LT, LEQ, EQ, GEQ, GT
    array_sep.add(s1);

//    Function f_sep2(x1, x2, x3, pow(x1-pt_xy, 2)+pow(x2-pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
//    SepFwdBwd s2(f_sep2, GEQ); // LT, LEQ, EQ, GEQ, GT
//    array_sep.add(s2);

//    Function f_sep3(x1, x2, x3, pow(x1+pt_xy, 2)+pow(x2+pt_xy, 2)+pow(x3-pt_z, 2)-pow(r, 2));
//    SepFwdBwd s3(f_sep3, GEQ); // LT, LEQ, EQ, GEQ, GT
//    array_sep.add(s3);

//    SepInter sep_total(array_sep);
//    dom.set_sep(&sep_total);

    // ****** Dynamics ******* //
//    ibex::Interval b = ibex::Interval(0.32899);
//    ibex::Interval b = ibex::Interval(0.208186);
    ibex::Interval b = ibex::Interval(0.1998);

    ibex::Function f(x, y, z, Return(sin(y)-b*x,
                                     sin(z)-b*y,
                                     sin(x)-b*z));
    DynamicsFunction dyn(&f, FWD_BWD);

    // ******* Maze ********* //
    invariant::MazePPL maze(&dom, &dyn);
    maze.set_enable_contraction_limit(true);
    maze.set_contraction_limit(20);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
        maze.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VtkMazePPL vtkMaze3D("thomas");
    vtkMaze3D.show_maze(&maze);

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-1.7);
    //    position_info[1] = ibex::Interval(1);
    //    v_maze.get_room_info(&maze, position_info);

}
