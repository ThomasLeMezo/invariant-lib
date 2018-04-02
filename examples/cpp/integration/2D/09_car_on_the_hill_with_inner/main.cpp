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
    space[0] = ibex::Interval(-2, 5);
    space[1] = ibex::Interval(-3, 3);

    invariant::SmartSubPaving<> paving(space);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 1.0;

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    Function f_sep_outer(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, LEQ); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    SepNot s_inner(s_outer);
    dom_inner.set_sep(&s_inner);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //
    double u= 1.2;
    ibex::Function f(x1, x2, Return(x2,
                                    -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-u, u)));
    Dynamics_Function dyn(&f, FWD);

    ibex::Function f1(x1, x2, -Return(x2,
                                      -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(-u)));
    ibex::Function f2(x1, x2, -Return(x2,
                                      -9.81*sin((1.1*sin(1.2*x1)-1.2*sin(1.1*x1))/2.0)-0.7*x2+ibex::Interval(u)));
    Dynamics_Function dyn_u(&f1, &f2, BWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_u);

    VibesMaze v_maze_debug("SmartSubPaving_debug", &maze_inner);
    v_maze_debug.setProperties(0, 0, 1024, 1024);
    string test;

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    omp_set_num_threads(1);
    for(int i=0; i<10; i++){
        cout << i << endl;
        paving.bisect();
        maze_outer.contract();
        if(i<9)
            maze_inner.contract();
        else{
            maze_inner.contract(1900);
//            for(int j=1700; j<1900; j+=10){
//                maze_inner.contract(j);
//                vibes::clearFigure(v_maze_debug.name());
//                v_maze_debug.show();
//                v_maze_debug.drawCircle(0.0, 0.0, 1, "red[]");
//                v_maze_debug.setAxis(-2, -0.5, -1.6, -0.3);
//                cin >> test;
//            }
        }
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("SmartSubPaving"/*, &maze_outer*/, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();
    //    v_maze.drawCircle(0.0, 0.0, 1, "black[red]");
    v_maze.drawCircle(0.0, 0.0, 1, "red[]");

        IntervalVector position_info(2);
        position_info[0] = ibex::Interval(-1.45);
        position_info[1] = ibex::Interval(-1);
        v_maze.setProperties(0, 0, 512, 512);
        v_maze.show_room_info(&maze_inner, position_info);


    vibes::endDrawing();
}
