#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    int iterations = 15;
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-4,4);
    space[1] = ibex::Interval(-4,4);
    invariant::SmartSubPaving<> paving(space);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn(&f, FWD);

    ibex::Function f_bwd(x1, x2, -Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn_bwd(&f_bwd, BWD);

    // ****** Maze A *******
    invariant::Domain<> dom_A(&paving, FULL_WALL);
    dom_A.set_border_path_in(false);
    dom_A.set_border_path_out(false);
    double xc_1, yc_1, r_1;
    xc_1 = 0.8; yc_1 = 1.3; r_1 = 0.2;
    Function f_sep_A(x1, x2, pow(x1-xc_1, 2)+pow(x2-yc_1, 2)-pow(r_1, 2));
    SepFwdBwd s_A(f_sep_A, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_A.set_sep(&s_A);

    invariant::Maze<> maze_A(&dom_A, &dyn);

    // ****** Maze A inner *******
    invariant::Domain<> dom_A_inner(&paving, FULL_DOOR);
    dom_A_inner.set_border_path_in(true);
    dom_A_inner.set_border_path_out(true);
    SepFwdBwd s_A_inner(f_sep_A, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_A_inner.set_sep(&s_A_inner);

    invariant::Maze<> maze_A_inner(&dom_A_inner, &dyn_bwd);

    // ****** Maze B *******
    invariant::Domain<> dom_B(&paving, FULL_WALL);
    dom_B.set_border_path_in(false);
    dom_B.set_border_path_out(false);
    IntervalVector box_B(2);
    box_B[0] = ibex::Interval(0.8, 1.6);
    box_B[1] = ibex::Interval(-0.7, -0.5);
    Function f_sep_B(x1, x2, Return(x1, x2));
    SepFwdBwd s_B(f_sep_B, box_B);
    dom_B.set_sep(&s_B);

    invariant::Maze<> maze_B(&dom_B, &dyn);

    dom_B.add_maze_inter(&maze_A);
//    dom_A.add_maze_inter(&maze_B);
//    dom_A_inner.add_maze_union(&maze_B);

    double time_start = omp_get_wtime();
    for(int i=0; i<iterations; i++){
        cout << i << "/" << iterations-1 << endl;
        paving.bisect();
        maze_A.contract();
        maze_A_inner.contract();
        maze_B.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_mazeA("graph_A", &maze_A, &maze_A_inner);
    v_mazeA.setProperties(0, 0, 512, 512);
    v_mazeA.show();
    v_mazeA.drawCircle(xc_1, yc_1, r_1, "r[]");

    VibesMaze v_mazeB("graph_B", &maze_B);
    v_mazeB.setProperties(600, 0, 512, 512);
    v_mazeB.show();
    v_mazeB.drawBox(box_B, "r[]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-2);
//    position_info[1] = ibex::Interval(4);
//    v_maze.get_room_info(&maze, position_info);

    vibes::endDrawing();
}
