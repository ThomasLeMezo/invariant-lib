#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex/ibex_SepFwdBwd.h"
#include "ibex/ibex_SepInter.h"
#include "ibex/ibex_SepUnion.h"

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
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);

    double x1_c, x2_c, r;
    x1_c = 1.0;
    x2_c = 0.0;
    r = 0.4;
    Function f_sep1(x, sqr(x[0]-x1_c)-sqr(r));
    Function f_sep2(x, sqr(x[1]-x2_c)-sqr(r));

    double x1_c2, x2_c2, r2;
    x1_c2 = -0.8;
    x2_c2 = 0.5;
    r2 = 0.2;
    Function f2_sep1(x, sqr(x[0]-x1_c2)-sqr(r2));
    Function f2_sep2(x, sqr(x[1]-x2_c2)-sqr(r2));

    invariant::Domain<> dom_outer(&paving, FULL_WALL);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    SepFwdBwd s1_outer1(f_sep1, LEQ); // LT, LEQ, EQ, GEQ, GT)
    SepFwdBwd s1_outer2(f_sep2, LEQ); // LT, LEQ, EQ, GEQ, GT)
    SepInter s1_outer(s1_outer1, s1_outer2);
    SepFwdBwd s2_outer1(f2_sep1, LEQ); // LT, LEQ, EQ, GEQ, GT)
    SepFwdBwd s2_outer2(f2_sep2, LEQ); // LT, LEQ, EQ, GEQ, GT)
    SepInter s2_outer(s2_outer1, s2_outer2);
    SepUnion s_outer(s1_outer, s2_outer);
    dom_outer.set_sep_output(&s_outer);

    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);
    SepFwdBwd s1_inner1(f_sep1, GEQ); // LT, LEQ, EQ, GEQ, GT)
    SepFwdBwd s1_inner2(f_sep2, GEQ); // LT, LEQ, EQ, GEQ, GT)
    SepUnion s1_inner(s1_inner1, s1_inner2);
    SepFwdBwd s2_inner1(f2_sep1, GEQ); // LT, LEQ, EQ, GEQ, GT)
    SepFwdBwd s2_inner2(f2_sep2, GEQ); // LT, LEQ, EQ, GEQ, GT)
    SepUnion s2_inner(s2_inner1, s2_inner2);
    SepInter s_inner(s1_inner, s2_inner);
    dom_inner.set_sep_input(&s_inner);

    // ****** Dynamics ******* //
    ibex::Function f(x, -Return(x[1],
                                    (1.0*(1.0-pow(x[0], 2))*x[1]-x[0])));
    Dynamics_Function dyn(&f, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_outer.init();
    maze_inner.init();
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " - " << maze_outer.contract() << " - " << paving.size() << endl;
        cout << i << " - " << maze_inner.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze("Van Der Pol 2 Basin", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.show();

//    VibesMaze v_maze_inner("graph_inner",&maze_inner, VibesMaze::VIBES_MAZE_INNER);
//    v_maze_inner.setProperties(0, 0, 512, 512);
//    v_maze_inner.show();

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.4);
//    position_info[1] = ibex::Interval(1.34);
//    v_maze.get_room_info(&maze_inner, position_info);

//    position_info[0] = ibex::Interval(-0.34);
//    position_info[1] = ibex::Interval(1.34);
//    v_maze.get_room_info(&maze_inner, position_info);

//    v_maze.drawCircle(x1_c, x2_c, r, "black[red]");
    v_maze.drawBox(x1_c-r, x1_c+r,x2_c-r, x2_c+r, "black[red]");
    v_maze.drawBox(x1_c2-r2, x1_c2+r2,x2_c2-r2, x2_c2+r2, "black[red]");
    vibes::saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/van_der_pol2_basin.svg", "Van Der Pol 2 Basin");
    vibes::endDrawing();

}
