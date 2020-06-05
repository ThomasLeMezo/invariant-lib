#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

#include "language.h"

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
    space[0] = ibex::Interval(0,12);
    space[1] = ibex::Interval(-20, 20);

    // ****** Domain *******
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 10.0;
    x2_c = 0.0;
    r = 1.0;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LT); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(x2,-30-1.0*x2));
    DynamicsFunction dyn(&f, FWD);

    // Hybrid
    ibex::Function f_sep_guard(x1, x2, x1-0.01*pow(x2, 2));
    ibex::SepFwdBwd sep_guard(f_sep_guard, EQ);

    ibex::SepFwdBwd sep_zero(f_sep_guard, GEQ);
    dom.set_sep_zero(&sep_zero);

    ibex::Function f_reset_pos(x1, x2, Return(x1, -0.75*x2));
    ibex::Function f_reset_neg(x1, x2, Return(x1, -0.75*x2));
    dyn.add_hybrid_condition(&sep_guard, &f_reset_pos, &f_reset_neg);

    // ******* Maze *********
    invariant::MazeIBEX maze(&dom, &dyn);

    omp_set_num_threads(1);
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
        maze.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;


    vibes::beginDrawing();
    string file_name = "hybrid_ball";
    string path_file = "/home/lemezoth/Pictures/";
    save_maze_image(&maze, path_file, file_name);
    vibes::endDrawing();


//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(0.05);
//    position_info[1] = ibex::Interval(-5.8);
//    v_maze.show_room_info(&maze, position_info);
}
