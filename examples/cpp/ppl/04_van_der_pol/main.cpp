#include "ibex_SepFwdBwd.h"
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
    space[0] = ibex::Interval(-6, 6);
    space[1] = ibex::Interval(-6, 6);
    space[2] = ibex::Interval(0,20);

    SmartSubPavingPPL paving(space);

    // ****** Domain Outer ******* //
    DomainPPL dom_outer(&paving, FULL_WALL);

    double x1_c, x2_c, x3_c, r;
    x1_c = 2.0;
    x2_c = 2.0;
    x3_c = 0.0;
    r = 0.1;
    IntervalVector initial_condition(3);
    initial_condition[0] = ibex::Interval(x1_c-r, x1_c+r);
    initial_condition[1] = ibex::Interval(x2_c-r/2.0, x2_c+r/2.0);
    initial_condition[2] = ibex::Interval(x3_c-r/2.0, x3_c+r/2.0);

    Function f_id(x1, x2, x3, Return(x1, x2, x3));
    SepFwdBwd s(f_id, initial_condition);

//    Function f_sep(x1, x2, x3, pow(x1-x1_c, 2)+pow(x2-x2_c, 2) + pow(x3-x3_c, 2) - pow(r, 2));
//    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep(&s);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Dynamics Outer & Inner ******* //
    ibex::Function f(x1, x2, x3, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1), pow(x2, 2)+ pow((1.0*(1.0-pow(x1, 2))*x2-x1), 2)));
    Dynamics_Function dyn_outer(&f, FWD);

    // ******* Mazes ********* //
    MazePPL maze_outer(&dom_outer, &dyn_outer);
    maze_outer.set_widening_limit(10);
    vector<double> ratio_bisection1 = {8.0, 8.0, 1.0};
    vector<double> ratio_bisection2 = {1.0, 1.0, 1.0};
    paving.set_ratio_bisection(ratio_bisection1);

    IntervalVector bounding_box(3);
    bounding_box[0] = ibex::Interval(-2, 4);
    bounding_box[1] = ibex::Interval(-4, 4);
    bounding_box[2] = ibex::Interval(0, 20);

    VtkMazePPL vtkMazePPL("VanDerPolPPL");

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);
    for(int i=0; i<20; i++){
        paving.bisect();
        cout << i << endl;

        maze_outer.get_domain()->set_init(FULL_WALL);
        maze_outer.set_enable_contract_domain(true);
        maze_outer.contract();

        maze_outer.get_domain()->set_init(FULL_DOOR);
        maze_outer.reset_nb_operations();
        maze_outer.set_enable_contract_domain(false);
        maze_outer.contract(10000);

        if(maze_outer.get_bounding_box().is_interior_subset(bounding_box)){
            paving.set_ratio_bisection(ratio_bisection2);
            paving.set_enable_bisection_strategy(2, true);
            paving.set_bisection_strategy_slice(2, 0.1);
        }

        cout << " - paving = " << paving.size() << endl;
        vtkMazePPL.show_maze(&maze_outer);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;
    cout << paving << endl;

//    IntervalVector position_info(3);
//    position_info[0] = ibex::Interval(0.0);
//    position_info[1] = ibex::Interval(0.0);
//    position_info[2] = ibex::Interval(0.5);
//    vtkMazePPL.show_room_info(&maze_outer, position_info);
}

