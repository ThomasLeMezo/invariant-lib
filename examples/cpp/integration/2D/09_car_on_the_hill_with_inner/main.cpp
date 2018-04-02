#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"
#include "booleantreenot.h"
#include "booleantreeinter.h"

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
    space[0] = ibex::Interval(-2, 13.0);
    space[1] = ibex::Interval(-3, 5);

    invariant::SmartSubPaving<> paving(space);

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);

    double x1_c, x2_c, r;
//    x1_c = 3.0;
//    x2_c = 2.0;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 1.0;

    IntervalVector box_init(2);
    box_init[0] = ibex::Interval(-1, 1);
    box_init[1] = ibex::Interval(-1, 1);

    Function f_sep_outer(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_WALL);

    Function f_sep_inner(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, LT); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep(&s_inner);

    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(false);

    // ****** Dynamics Outer & Inner ******* //
//    ibex::Interval u(1.2);
//    ibex::Interval u(-1.2);
    double u= 1.2;
    ibex::Function f(x1, x2, Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-u, u)));
    Dynamics_Function dyn(&f, FWD);

    ibex::Function f1(x1, x2, Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-u)));
    ibex::Function f2(x1, x2, Return(x2,
                                     -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(u)));
    Dynamics_Function dyn_u(&f1, &f2, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_u);

    BooleanTreeNotIBEX tree_inner(&maze_inner);
    BooleanTreeUnionIBEX tree_removing(&maze_outer, &tree_inner);
    maze_outer.set_boolean_tree_removing(&tree_removing);
    maze_inner.set_boolean_tree_removing(&tree_removing);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    
//    omp_set_num_threads(1);
    for(int i=0; i<18; i++){
        cout << i  << endl;
        paving.bisect();
        maze_outer.contract();
        maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner, true);
//    VibesMaze v_maze("SmartSubPaving", &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();

//    v_maze.drawCircle(0.0, 0.0, 1, "black[red]");
    v_maze.drawCircle(0.0, 0.0, 1, "red[]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(3.8);
//    position_info[1] = ibex::Interval(0.4);
//    v_maze.setProperties(0, 0, 512, 512);
//    v_maze.show_room_info(&maze_inner, position_info);


    vibes::endDrawing();
}
