#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

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
    ibex::Variable x(2);

    IntervalVector space(2);
    space[0] = ibex::Interval(-2, 2);
    space[1] = ibex::Interval(-1.5, 1.5);

//    space[0] = ibex::Interval(-2, 11);
//    space[1] = ibex::Interval(-7, 7);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);

    ibex::IntervalVector box(2);
    box[0] = ibex::Interval(-0.5, 0);
    box[1] = ibex::Interval(-0.5, 0.5);
    Function f_id("x[2]", "(x[0], x[1])");
    SepFwdBwd s_outer(f_id, box);
    SepNot s_inner(s_outer);

    ibex::IntervalVector box_remove(2);
    box_remove[0] = ibex::Interval(0,5);
    box_remove[1] = ibex::Interval(-1, 1);
    Function f_remove("x[2]", "(x[0], x[1])");
    SepFwdBwd s_remove(f_remove, box_remove);
    SepNot s_remove_not(s_remove);

    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);
    dom_outer.set_sep_output(&s_outer);
    dom_outer.set_sep_zero(&s_remove_not);

    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    dom_inner.set_sep_input(&s_inner);
    dom_inner.set_sep_zero(&s_remove_not);

    // ****** Dynamics ******* //
    double k = 15; //2.5
    ibex::Interval x_err(-0.1, 0.1);
    ibex::Interval y_err(-0.1, 0.1);
    ibex::Interval consigne_err(0.0);

    ibex::Function f(x, -Return(consigne_err+ 50.0*(k*sign(atan2((x[1]+y_err), (x[0]+x_err)))*((-1-2*(x[0]+x_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err)))*(x[1]+y_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err)))                                            +tanh((k*(1+(x[0]+x_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err)))-sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err))))*(x[0]+x_err)),
                                consigne_err + 50.0*(k*sign(atan2((x[1]+y_err), (x[0]+x_err)))*((x[0]+x_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err))*(1+(x[0]+x_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err)))-(x[1]+y_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err))*(x[1]+y_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err)))  +tanh((k*(1+(x[0]+x_err)/sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err)))-sqrt((x[0]+x_err)*(x[0]+x_err)+(x[1]+y_err)*(x[1]+y_err))))*(x[1]+y_err))));
    Dynamics_Function dyn(&f,
                          FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    
    for(int i=0; i<17; i++){
        paving.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("Cardioide", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    v_maze.drawBox(box_remove, "g[]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.4);
//    position_info[1] = ibex::Interval(1.34);
//    v_maze.get_room_info(&maze_inner, position_info);


//    ibex::Function f_test(x,atan2(x[1], x[0]));
//    ibex::IntervalVector box_test(2);
//    box_test[0] = ibex::Interval(-2, -1);
//    box_test[1] = ibex::Interval(-0.1, 0.1);
//    cout << "box_test -> " << f_test.eval_vector(box_test) << endl;

}
