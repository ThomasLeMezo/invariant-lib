#include "ibex_SepFwdBwd.h"

#include "eulerianmaze.h"
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
    space[0] = ibex::Interval(-4,4);
    space[1] = ibex::Interval(-4,4);
    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));

    // Sep A
    double xc_1, yc_1, r_1;
    xc_1 = 0.8; yc_1 = 1.3; r_1 = 0.2;
    Function f_sep_A(x1, x2, pow(x1-xc_1, 2)+pow(x2-yc_1, 2)-pow(r_1, 2));
    SepFwdBwd sep_A(f_sep_A, LEQ); // LT, LEQ, EQ, GEQ, GT)

    // Sep B
    IntervalVector box_B(2);
    box_B[0] = ibex::Interval(1.0, 1.45);
    box_B[1] = ibex::Interval(-0.2, 0);
    Function f_sep_B(x1, x2, Return(x1, x2));
    SepFwdBwd sep_B(f_sep_B, box_B);

    // Sep C
    IntervalVector box_C(2);
    box_C[0] = ibex::Interval(0.8, 1.2);
    box_C[1] = ibex::Interval(-1.5, -1.1);
    Function f_sep_C(x1, x2, Return(x1, x2));
    SepFwdBwd sep_C(f_sep_C, box_C);

    // Eulerian
    EulerianMaze<> eulerian_maze(space, &f, &sep_A, &sep_B, &sep_C, true);

    double time_start = omp_get_wtime();
    //    omp_set_num_threads(1);
    for(int i=0; i<20; i++){
        cout << i << endl;
        eulerian_maze.bisect();
        eulerian_maze.contract(1);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

//    for(size_t i=0; i<eulerian_maze.get_number_maze(); i++){
//        ostringstream name;
//        name << "graph_" << i;
//        VibesMaze v_maze(name.str(), eulerian_maze.get_maze_outer(i), eulerian_maze.get_maze_inner(i));
//        v_maze.setProperties(0, 0, 512, 512);
//        v_maze.show();

//        v_maze.drawBox(box_C, "r[]");
//        v_maze.drawBox(box_B, "r[]");
//        v_maze.drawCircle(xc_1, yc_1, r_1, "r[]");

////        if(i==3){
////            IntervalVector position_info(2);
////            position_info[0] = ibex::Interval(1.1);
////            position_info[1] = ibex::Interval(0.6);
////            v_maze.show_room_info(eulerian_maze.get_maze_outer(i), position_info);
////            if(eulerian_maze.get_maze_outer(i)->get_dynamics()->get_sens()==BWD)
////                cout << "BWD detected" << endl;
////        }
//    }
    VibesMaze v_maze_eulerian("Eulerian", &eulerian_maze);
    v_maze_eulerian.setProperties(0, 0, 512, 512);
    v_maze_eulerian.show();
    v_maze_eulerian.drawBox(box_C, "r[]");
    v_maze_eulerian.drawBox(box_B, "r[]");
    v_maze_eulerian.drawCircle(xc_1, yc_1, r_1, "r[]");

    vibes::endDrawing();
}
