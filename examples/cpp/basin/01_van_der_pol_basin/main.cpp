#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
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
    space[0] = ibex::Interval(-4,4);
    space[1] = ibex::Interval(-4,4);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);

    double x1_c, x2_c, r;
    x1_c = 0.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep(x, pow(x[0]-x1_c, 2)+pow(x[1]-x2_c, 2)-pow(r, 2));

    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);
    SepFwdBwd s_outer(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_outer.set_sep_output(&s_outer);

    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    SepFwdBwd s_inner(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
//    ibex::Function f(x, Return(x[1],
//                                    (1.0*(1.0-pow(x[0], 2))*x[1]-x[0])));
    ibex::Function f("x[2]", "(x[1], (1-x[0]^2)*x[1]-x[0])");
    DynamicsFunction dyn(&f, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Visu ********* //

    vibes::beginDrawing();
    VibesMaze v_maze("Van Der Pol Basin", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    vector<double> memory_time, memory_volume_outer, memory_volume_inner;

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    
    for(int i=0; i<21; i++){ // 13
        cout << i << endl;
        double time_local = omp_get_wtime();

        paving.bisect();
        maze_outer.contract();
        maze_inner.contract();

        // Stat
        double t = omp_get_wtime() - time_local;
        double v_outer =  v_maze.get_volume();
        double v_inner =  v_maze.get_volume(true);
        v_maze.add_stat(i, t, v_outer, v_inner);
    }
    cout << "TIME TOTAL = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    v_maze.show();
    v_maze.save_stat_to_file("stat.txt");



    v_maze.drawCircle(x1_c, x2_c, r, "black[red]");
    vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/van_der_pol_basin.svg", "Van Der Pol Basin");
    vibes::endDrawing();

}
