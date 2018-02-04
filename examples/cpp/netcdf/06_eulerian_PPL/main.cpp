#include "ibex_SepFwdBwd.h"
#include <iostream>
#include "previmer3d.h"
#include "dynamics_function.h"
#include "maze.h"
#include <ibex.h>
#include <math.h>
#include "eulerianmaze.h"

#include "vtkMaze3D.h"
#include "vtkmazeppl.h"

using namespace std;
using namespace invariant;
using namespace ibex;

int main(int argc, char *argv[])
{
    ibex::Variable t, x, y;
    Function f_id(t, x, y, Return(t, x, y));

    // ****** Dynamics ******* //
//    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/file_test.xml");
    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");

    array<array<size_t, 2>, 2> grid_limits; // X, Y limit data loading
    grid_limits[0][0] = 80; grid_limits[0][1] = 280; // max = 584
    grid_limits[1][0] = 380; grid_limits[1][1] = 580; // max = 754

//    PreviMer3D pm3d = PreviMer3D(sources_xml, grid_limits);
    PreviMer3D pm3d = PreviMer3D("PreviMer3D.data");

    // ****** Domain ******* //
    IntervalVector search_space(3);
    search_space = pm3d.get_search_space();
    cout << "Search_space = " << search_space << endl;

    // ** A ** //
    double t_c, x_c, y_c, r_spatial, r_time;
    t_c = search_space[0].lb();
    x_c = 137 * pm3d.get_grid_conversion(1);
    y_c = 477 * pm3d.get_grid_conversion(2);
    r_spatial = 1000.0; // in m
    r_time = 5*60.0; // in s

    IntervalVector initial_condition(3);
    initial_condition[0] = ibex::Interval(t_c-r_time, t_c+r_time);
    initial_condition[1] = ibex::Interval(x_c-r_spatial, x_c+r_spatial);
    initial_condition[2] = ibex::Interval(y_c-r_spatial, y_c+r_spatial);
    SepFwdBwd s_A(f_id, initial_condition);

    // ** B ** //
    t_c = search_space[0].ub();
    initial_condition[0] = ibex::Interval(t_c-r_time, t_c+r_time);
    SepFwdBwd s_B(f_id, initial_condition);

    // ** C ** //
    t_c = 23000;
    x_c = 46100;
    y_c = 125000;
    r_time = 5000.0; // in s
    r_spatial = 1500.0; // in m
    initial_condition[0] = ibex::Interval(t_c-r_time, t_c+r_time);
    initial_condition[1] = ibex::Interval(x_c-r_spatial, x_c+r_spatial);
    initial_condition[2] = ibex::Interval(y_c-r_spatial, y_c+r_spatial);
    SepFwdBwd s_C(f_id, initial_condition);

    EulerianMazePPL eulerian_maze(search_space, &pm3d, &s_A, &s_C, &s_B, false, false);

    // Paving bisection strategy
    const std::vector<double> limit_bisection = {5*60, 250.0/4.0, 250.0/4.0};
    eulerian_maze.get_paving()->set_limit_bisection(limit_bisection);
    eulerian_maze.get_paving()->set_enable_bisection_strategy(0, BISECTION_STANDARD);
    eulerian_maze.get_paving()->set_bisection_strategy_slice(0, 900*3); // 3?

    //Maze contraction strategy
    for(MazePPL* maze:eulerian_maze.get_maze_outer()){
        maze->set_widening_limit(10); // 10 ?
        maze->set_enable_contraction_limit(true);
        maze->set_enable_contract_vector_field(true);
    }
//    for(MazePPL* maze:eulerian_maze.get_maze_inner()){
//        maze->set_enable_contraction_limit(true);
//        maze->set_contraction_limit(15); // 15 ?
//        maze->set_enable_contract_vector_field(true);
//    }

    double time_start = omp_get_wtime();
    VtkMazePPL vtkMazePPL_A("PrevimerPPL_eulerian_A");
    VtkMazePPL vtkMazePPL_B("PrevimerPPL_eulerian_B");
    VtkMazePPL vtkMazePPL_C_FWD("PrevimerPPL_eulerian_C_FWD");
    VtkMazePPL vtkMazePPL_C_BWD("PrevimerPPL_eulerian_C_BWD");
//    VtkMazePPL vtkMazePPL_B_inner("PrevimerPPL_eulerian_B_inner");

    //    omp_set_num_threads(1);
    for(int i=0; i<30; i++){
        std::time_t t_now = std::time(nullptr);
        cout << i << " - " << std::ctime(&t_now);
        eulerian_maze.bisect();
        eulerian_maze.contract(1);

        vtkMazePPL_A.show_maze(eulerian_maze.get_maze_outer(0));
        vtkMazePPL_A.show_subpaving(eulerian_maze.get_maze_outer(0));
        vtkMazePPL_C_FWD.show_maze(eulerian_maze.get_maze_outer(1));
        vtkMazePPL_B.show_maze(eulerian_maze.get_maze_outer(2));
        vtkMazePPL_C_BWD.show_maze(eulerian_maze.get_maze_outer(3));
//        vtkMazePPL_B_inner.show_maze(eulerian_maze.get_maze_inner(1));
    }

    cout << "TIME = " << omp_get_wtime() - time_start << "s" << endl;

    IntervalVector position(3);
    position[0] = ibex::Interval(28000); // 450, 900
    position[1] = ibex::Interval(47600); // 37304, 37980
    position[2] = ibex::Interval(126500); // 119766, 120469
//    vtkMazePPL_A.show_room_info(eulerian_maze.get_maze_outer(0), position);

    return 0;
}
