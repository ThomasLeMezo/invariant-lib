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
    box_B[0] = ibex::Interval(1.2, 1.6);
    box_B[1] = ibex::Interval(-0.2, 0);
    Function f_sep_B(x1, x2, Return(x1, x2));
    SepFwdBwd sep_B(f_sep_B, box_B);

    // Eulerian
    EulerianMaze<> eulerian_maze(space, &f, &sep_A, &sep_B);

    double time_start = omp_get_wtime();
    //    omp_set_num_threads(1);
    for(int i=0; i<15; i++){
        cout << i << endl;
        eulerian_maze.bisect();
        eulerian_maze.contract(1);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    VibesMaze v_mazeA("graph_A", eulerian_maze.get_maze_outer(0), eulerian_maze.get_maze_inner(0));
    v_mazeA.setProperties(0, 0, 512, 512);
    v_mazeA.show();
    v_mazeA.drawCircle(xc_1, yc_1, r_1, "r[]");
    v_mazeA.drawBox(box_B, "g[]");

    VibesMaze v_mazeB("graph_B", eulerian_maze.get_maze_outer(1), eulerian_maze.get_maze_inner(1));
    v_mazeB.setProperties(600, 0, 512, 512);
    v_mazeB.show();
    v_mazeB.drawBox(box_B, "r[]");
    v_mazeB.drawCircle(xc_1, yc_1, r_1, "g[]");

    vibes::endDrawing();
}
