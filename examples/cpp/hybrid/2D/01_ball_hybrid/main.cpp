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

double x1_c, x2_c, r;

void save_maze_image(invariant::Maze<> &maze, const int step, std::string directory){
    std::stringstream fig_name;
    fig_name << "hybrid" << std::setfill('0') << std::setw(5) << step;
    VibesMaze v_maze(fig_name.str(), &maze);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);

    v_maze.set_ipe_ratio(112,63,false);
    v_maze.set_axis_limits(0, 0.5, -5, 1);
    v_maze.set_enable_white_boundary(false);
    v_maze.set_thickness_pen_factor(1e-4);
    v_maze.set_enable_vibes(false);
    v_maze.set_number_digits_x(1);
    v_maze.set_number_digits_y(0);
    v_maze.show();

    v_maze.draw_axis("x_1", "x_2");
    v_maze.draw_text(std::to_string(step), 0.0, 3.2);
    v_maze.drawCircle(x1_c, x2_c, r, "red", "red");

    v_maze.saveIpe(directory);
}

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

    // VibesMaze v_maze("hybrid_ball", &maze);
    // v_maze.setProperties(0, 0, 1000, 800);
    // v_maze.set_enable_cone(false);
    // v_maze.show();
    // v_maze.drawCircle(x1_c, x2_c, r, "red", "red");
    // v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

    string file_name = "hybrid_ball";
    string path_file = "/home/lemezoth/Pictures/";
    save_maze_image(&maze, path_file, file_name);
    vibes::endDrawing();

    save_maze_image(maze,0, "/home/lemezoth/");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(0.05);
//    position_info[1] = ibex::Interval(-5.8);
//    v_maze.show_room_info(&maze, position_info);
}
