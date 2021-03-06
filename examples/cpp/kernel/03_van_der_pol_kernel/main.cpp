#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

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
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-4, 4);

    invariant::SmartSubPaving<> paving(space);

    double r = 1.0;

    // ****** Domain Outer ******* //
    invariant::Domain<> dom_outer(&paving, FULL_DOOR);

    Function f_sep_outer(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain<> dom_inner(&paving, FULL_WALL);

    Function f_sep_inner(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, LEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep_input(&s_inner);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    // ****** Dynamics Outer ******* //
    ibex::IntervalVector u(2);
    u[0] = ibex::Interval(-0.2, 0.2);
    u[1] = ibex::Interval(-0.5, 0.5);
    ibex::Function f_outer(x1, x2, -(Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1))+u));
    DynamicsFunction dyn_outer(&f_outer, FWD_BWD);

    // ****** Dynamics Inner ******* //
    ibex::Function f_inner1(x1, x2, Return(x2+u[0].ub(),
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+u[1].lb()));
    ibex::Function f_inner2(x1, x2, Return(x2+u[0].ub(),
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+u[1].ub()));
    ibex::Function f_inner3(x1, x2, Return(x2+u[0].lb(),
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+u[1].lb()));
    ibex::Function f_inner4(x1, x2, Return(x2+u[0].lb(),
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)+u[1].ub()));
    vector<Function *> f_list_inner;
    f_list_inner.push_back(&f_inner1);
    f_list_inner.push_back(&f_inner2);
    f_list_inner.push_back(&f_inner3);
    f_list_inner.push_back(&f_inner4);
    DynamicsFunction dyn_inner(f_list_inner, FWD);

    // ******* Mazes ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
//    omp_set_num_threads(1);
    for(int i=0; i<15; i++){
        paving.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("van_der_pol_kernel", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);

    // For IPE
    v_maze.set_ipe_ratio(150,150,false);
    ibex::IntervalVector bounding_box = maze_outer.get_bounding_box();
    v_maze.set_axis_limits(bounding_box[0].lb(), bounding_box[0].diam()/6., bounding_box[1].lb(), bounding_box[1].diam()/6.);
    v_maze.set_enable_white_boundary(false);
    v_maze.set_thickness_pen_factor(1e-4);
    v_maze.set_enable_vibes(true);
    v_maze.set_number_digits_x(1);
    v_maze.set_number_digits_y(1);

    v_maze.show();
    v_maze.draw_axis("x_1", "x_2");
    v_maze.set_current_layer("data");

    v_maze.drawCircle(0,0,r,"red","");
    v_maze.savePdf("");
//    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

//    VibesMaze v_maze2("graph_2", &maze_inner);
//    v_maze2.setProperties(0, 0, 1024, 1024);
//    v_maze2.show();

//    position_info[0] = ibex::Interval(1.745);
//    position_info[1] = ibex::Interval(-0.725);
//    v_maze2.get_room_info(&maze_inner, position_info);

    vibes::endDrawing();
}
