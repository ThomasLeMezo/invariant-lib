#include "ibex_SepFwdBwd.h"
#include <iostream>
#include "previmer3d.h"
#include "maze.h"
#include <ibex.h>
#include <math.h>

#include "vtkMaze3D.h"
#include "vtkmazeppl.h"

using namespace std;
using namespace invariant;
using namespace ibex;

int main(int argc, char *argv[])
{
    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/file_test.xml");
//    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");

    IntervalVector search_space(3);

    array<array<size_t, 2>, 2> grid_limits; // X, Y limit data loading
    grid_limits[0][0] = 80; grid_limits[0][1] = 280;
    grid_limits[1][0] = 380; grid_limits[1][1] = 580;
//    grid_limits[0][0] = 0; grid_limits[0][1] = 584;
//    grid_limits[1][0] = 0; grid_limits[1][1] = 754;

    // ****** Dynamics *******
    double time_start_PM = omp_get_wtime();
    PreviMer3D pm3d = PreviMer3D(sources_xml, grid_limits);
//    PreviMer3D pm3d = PreviMer3D("PreviMer3D.data");
    search_space = pm3d.get_search_space();
    cout << "TIME load PreviMer = " << omp_get_wtime() - time_start_PM << endl << endl;
    cout << "Search_space = " << search_space << endl;

/// **************** TEST 1 **************** ///
//    IntervalVector test_position(3);
//    test_position[0] = ibex::Interval(0, 14.0625);
//    test_position[1] = ibex::Interval(52468.8, 52611.3);
//    test_position[2] = ibex::Interval(99588.4, 99772.5);
//    vector<ibex::IntervalVector> result = pm3d.eval(test_position);
//    cout << "Result = " << endl;
//    for(IntervalVector &iv:result)
//        cout << iv << endl;

/// **************** TEST 2 **************** ///

//    monteCarlos(pm3d, 0, 130.4, 460.4);
//    monteCarlos(pm3d, 0, 130, 460);
//    monteCarlos(pm3d, 0, 131, 460);
//    monteCarlos(pm3d, 0, 131, 461);
//    monteCarlos(pm3d, 0, 130, 460);
//    monteCarlos(pm3d, 0, 130, 470);

#if 1
    // ****** Domain *******
//    invariant::SmartSubPavingPPL paving(search_space);
//    invariant::DomainPPL dom(&paving, FULL_WALL);
    invariant::SmartSubPavingIBEX paving(search_space);
    invariant::DomainIBEX dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    const std::vector<double> limit_bisection = {15*60-1, 250-1, 250-1};
    paving.set_limit_bisection(limit_bisection);

    double t_c, x_c, y_c, r;
    t_c = 0 * pm3d.get_grid_conversion(0);
    x_c = 137 * pm3d.get_grid_conversion(1);
    y_c = 477 * pm3d.get_grid_conversion(2);
    r = 50.0;
    cout << "Center of initial set = " << t_c << " " << x_c << " " << y_c << endl;
    IntervalVector initial_condition(3);
    initial_condition[0] = ibex::Interval(t_c-r, t_c+r);
    initial_condition[1] = ibex::Interval(x_c-r, x_c+r);
    initial_condition[2] = ibex::Interval(y_c-r, y_c+r);
    ibex::Variable t, x, y;
    Function f_id(t, x, y, Return(t, x, y));
    SepFwdBwd s(f_id, initial_condition);
    dom.set_sep(&s);

    // ******* Maze *********
//    invariant::MazePPL maze(&dom, &pm3d);
    invariant::MazeIBEX maze(&dom, &pm3d);

    cout << "Domain = " << search_space << endl;

    double time_start = omp_get_wtime();
//    VtkMazePPL vtkMazePPL("PrevimerPPL");
    VtkMaze3D vtkMaze3D("PrevimerIBEX");
    omp_set_num_threads(1);
    for(int i=0; i<30; i++){
        cout << i << endl;
        double time_start_bisection = omp_get_wtime();
        paving.bisect();
        cout << " => bisection : " << omp_get_wtime() - time_start_bisection << "s - " << paving.size() << endl;
        maze.contract();
//        vtkMazePPL.show_maze(&maze);
        vtkMaze3D.show_maze(&maze);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << "s" << endl;

    cout << paving << endl;

//    VtkMaze3D vtkMaze3D("Previmer", true);
////     vtkMaze3D.show_graph(&paving);
////    vtkMaze3D.show_maze(&maze);
//    vtkMaze3D.serialize_maze("current.maze", &maze);

//    IntervalVector position(3);
//    position[0] = ibex::Interval(t_c); // 450, 900
//    position[1] = ibex::Interval(x_c); // 37304, 37980
//    position[2] = ibex::Interval(y_c); // 119766, 120469
//    vtkMaze3D.show_room_info(&maze, position);

#endif

    return 0;
}
