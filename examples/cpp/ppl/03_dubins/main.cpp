#include "ibex_SepFwdBwd.h"
#include "ibex_SepNot.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
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
    ibex::Variable x, y, theta;

    IntervalVector space(3);
    space[0] = ibex::Interval(-10, 10);
    space[1] = ibex::Interval(-10, 10);
    space[2] = ibex::Interval::ZERO | ibex::Interval::TWO_PI;

    ibex::Interval v = ibex::Interval(0.1);
    ibex::Interval u = ibex::Interval(0.3);

    SmartSubPavingPPL paving(space);

    // ****** Domain Outer ******* //
    DomainPPL dom_outer(&paving, FULL_WALL);

//    double x_c, y_c, theta_c, r;
//    x_c = 0.0;
//    y_c = 0.0;
//    theta_c = 0.5;
//    r = 1.0;
////    r = 0.0;
//    Function f_sep_outer(x, y, theta, pow(x-x_c, 2)+pow(y-y_c, 2) + pow(theta-theta_c, 2) - pow(r, 2));
//    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT
//    dom_outer.set_sep(&s_outer);

    IntervalVector initial_condition(3);
    initial_condition[0] = ibex::Interval(-0.5, 0.5);
    initial_condition[1] = ibex::Interval(-0.5, 0.5);
    initial_condition[2] = ibex::Interval(0, 0.5);
    Function f_id(x, y, theta, Return(x, y, theta));
    SepFwdBwd s_outer(f_id, initial_condition);
    dom_outer.set_sep_input(&s_outer);

    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    DomainPPL dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    SepNot s_inner(s_outer);
    dom_inner.set_sep(&s_inner);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x, y, theta, Return(v*cos(theta),
                                            v*sin(theta),
                                            u));
    DynamicsFunction dyn(&f, FWD);

    // ******* Mazes ********* //
    MazePPL maze_outer(&dom_outer, &dyn);
    maze_outer.set_widening_limit(20);

    MazePPL maze_inner(&dom_inner, &dyn);
    maze_inner.set_enable_contraction_limit(true);
    maze_inner.set_contraction_limit(3);

    VtkMazePPL vtkMazePPL_outer("DubinsPPL_outer");
    VtkMazePPL vtkMazePPL_inner("DubinsPPL_inner");

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();

        maze_outer.get_domain()->set_init(FULL_WALL);
        maze_outer.set_enable_contract_domain(true);
        cout << " ==> Outer widening" << endl;
        maze_outer.contract();

        maze_outer.get_domain()->set_init(FULL_DOOR);
        maze_outer.reset_nb_operations();
        maze_outer.set_enable_contract_domain(false);
        cout << " ==> Outer back" << endl;
        maze_outer.contract(30000);
//        vtkMazePPL_outer.show_maze(&maze_outer);

        cout << " ==> Inner" << endl;
        maze_inner.contract(paving.size_active()*3);
//        vtkMazePPL_inner.show_maze(&maze_inner, "", true);

//        vtkMazePPL_outer.show_subpaving(&maze_outer);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;
    cout << paving << endl;

//    IntervalVector position_info(3);
//    position_info[0] = ibex::Interval(0.0);
//    position_info[1] = ibex::Interval(0.0);
//    position_info[2] = ibex::Interval(0.5);
//    vtkMazePPL.show_room_info(&maze_outer, position_info);
}

