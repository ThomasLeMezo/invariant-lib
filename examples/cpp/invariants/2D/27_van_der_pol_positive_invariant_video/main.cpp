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

#include <sstream>

using namespace std;
using namespace ibex;
using namespace invariant;

void save_maze_image(invariant::Maze<> &maze, const int step, std::string directory){
    std::stringstream fig_name;
    fig_name << "van_der_pol_positive" << std::setfill('0') << std::setw(5) << step;
    VibesMaze v_maze(fig_name.str(), &maze);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);

    v_maze.set_ipe_ratio(112,63,true);
    v_maze.set_axis_limits(-3, 1.0, -3, 1.0);
    v_maze.set_enable_white_boundary(false);
    v_maze.set_thickness_pen_factor(1e-4);
    v_maze.set_enable_vibes(false);
    v_maze.set_number_digits_x(0);
    v_maze.set_number_digits_y(0);
    v_maze.show();

    v_maze.draw_axis("x_1", "x_2");
    v_maze.draw_text(std::to_string(step), 0.0, 3.2);

    v_maze.saveIpe(directory);
}

void save_maze_image(invariant::Maze<> &maze_outer, invariant::Maze<> &maze_inner, const int step, std::string directory){
    std::stringstream fig_name;
    fig_name << "van_der_pol_positive" << std::setfill('0') << std::setw(5) << step;
    VibesMaze v_maze(fig_name.str(), &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);

    v_maze.set_ipe_ratio(112,63,true);
    v_maze.set_axis_limits(-3, 1.0, -3, 1.0);
    v_maze.set_enable_white_boundary(false);
    v_maze.set_thickness_pen_factor(1e-4);
    v_maze.set_enable_vibes(false);
    v_maze.set_number_digits_x(0);
    v_maze.set_number_digits_y(0);
    v_maze.show();

    v_maze.draw_axis("x_1", "x_2");
    v_maze.draw_text(std::to_string(step), 0.0, 3.2);

    v_maze.saveIpe(directory);
}

void example1(){
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);
    invariant::Domain<> dom(&subpaving, FULL_DOOR);

    dom.set_border_path_in(true);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, -Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    DynamicsFunction dyn(&f, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);

    // ******* Algorithm ********* //
    omp_set_num_threads(1);
    double time_start = omp_get_wtime();
    for(int i=0; i<11; i++){
        cout << i << endl;
        subpaving.bisect();
        if(i==10){
            for(size_t k=2; k<3000; k+=5){
                int nb_operation = maze.contract(k);
                cout << nb_operation << endl;
                if(nb_operation==0)
                    break;
                save_maze_image(maze, k, "/home/lemezoth/Videos/thesis/invariant/imgs/");
            }
        }
        else
            maze.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;
}

void example2(){
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(true);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                     (1.0*(1.0-pow(x1, 2))*x2-x1)));
    ibex::Function f2(x1, x2, Return(x2,
                                      (1.0*(1.0-pow(x1, 2))*x2-x1)));
    DynamicsFunction dyn_outer(&f, FWD); // Duplicate because of simultaneous access of f (semaphore on DynamicsFunction)
    DynamicsFunction dyn_inner(&f2, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    for(int i=0; i<20; i++){
        cout << i << endl;
        subpaving.bisect();
        maze_outer.contract();
        maze_inner.contract();
        save_maze_image(maze_outer, maze_inner, i, "/home/lemezoth/Videos/thesis/invariant/iterations/");
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;
}

int main(int argc, char *argv[])
{
    example1();
//    example2();
}
