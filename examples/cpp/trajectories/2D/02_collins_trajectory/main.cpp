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
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = Interval(-5,5);
    space[1] = Interval(-3.5,10);
    SmartSubPaving paving(space);

    // ****** MAZE A ******
    // Dynamics
    ibex::Function f_A(x1, x2, Return(2*x1-x1*x2,2*pow(x1,2)-x2));
    Dynamics_Function dynA(&f_A, FWD);

    // Domain
    invariant::Domain dom_A(&paving, FULL_WALL);
    dom_A.set_border_path_in(false);
    dom_A.set_border_path_out(false);
    Function f_sep_A(x1, x2, pow(x1, 2)+pow(x2+2, 2)-1);
    SepFwdBwd s_A(f_sep_A, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_A.set_sep(&s_A);

    // Maze
    Maze maze_A(&dom_A, &dynA);

    // ****** MAZE B ******
    // Dynamics
    ibex::Function f_B(x1, x2, -Return(2*x1-x1*x2,2*pow(x1,2)-x2));
    Dynamics_Function dynB(&f_B, FWD);

    // Domain
    invariant::Domain dom_B(&paving, FULL_WALL);
    dom_B.set_border_path_in(false);
    dom_B.set_border_path_out(false);
    Function f_sep_B(x1, x2, pow(x1, 2)+pow(x2-1, 2)-pow(9.0/100.0,2));
    SepFwdBwd s_B(f_sep_B, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_B.set_sep(&s_B);

    Maze maze_B(&dom_B, &dynB);

    dom_B.add_maze_inter(&maze_A);
    dom_A.add_maze_inter(&maze_B);

    double time_start = omp_get_wtime();

    for(int i=0; i<17; i++){
        cout << i << endl;
        paving.bisect();
        maze_A.contract();
        maze_B.contract();
        maze_A.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_mazeA("graph_A", &maze_A);
    v_mazeA.setProperties(0, 0, 512, 512);
    v_mazeA.show();
    vibes::drawCircle(0.0, -2.0, 1.0, "r[]");

    VibesMaze v_mazeB("graph_B", &maze_B);
    v_mazeB.setProperties(600, 0, 512, 512);
    v_mazeB.show();
    vibes::drawCircle(0.0, 1.0, 9.0/100.0, "r[]");

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-2);
//    position_info[1] = Interval(8);
//    v_mazeA.show_room_info(&maze_A, position_info);

}
