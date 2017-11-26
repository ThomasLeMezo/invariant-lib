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
    vector<Function *> f_list;
    f_list.push_back(&f);
    Dynamics_Function dyn(f_list, Dynamics::FWD);

    // ****** Domain & Maze *******
    invariant::Domain<> dom_A(&paving, invariant::Domain<>::FULL_WALL);
    dom_A.set_border_path_in(false);
    dom_A.set_border_path_out(false);
    double xc_1, yc_1, r_1;
    xc_1 = -1.0;
    yc_1 = 1.0;
    r_1 = 0.5;
    Function f_sep_A(x1, x2, pow(x1-xc_1, 2)+pow(x2-yc_1, 2)-pow(r_1, 2));
    SepFwdBwd s_A(f_sep_A, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_A.set_sep(&s_A);

    invariant::Maze<> maze_A(&dom_A, &dyn);

    invariant::Domain<> dom_B(&paving, invariant::Domain<>::FULL_WALL);
    dom_B.set_border_path_in(false);
    dom_B.set_border_path_out(false);
    IntervalVector box_B(2);
    box_B[0] = ibex::Interval(-0.5, 0.5);
    box_B[1] = ibex::Interval(-2.5, -1.5);
    Function f_sep_B(x1, x2, Return(x1, x2));
    SepFwdBwd s_B(f_sep_B, box_B);
    dom_B.set_sep(&s_B);

    invariant::Maze<> maze_B(&dom_B, &dyn);

    dom_B.add_maze_inter(&maze_A);
//    dom_A.add_maze_inter(&maze_B);

    double time_start = omp_get_wtime();
    for(int i=0; i<iterations; i++){
        cout << i << "/" << iterations-1 << endl;
        paving.bisect();
        maze_A.contract();
        maze_B.contract();
        maze_A.contract();
        maze_B.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_mazeA("graph_A", &maze_A);
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
