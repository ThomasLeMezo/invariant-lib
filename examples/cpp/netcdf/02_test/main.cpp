#include "ibex/ibex_SepFwdBwd.h"
#include <iostream>
#include "previmer3d.h"
#include "maze.h"
#include <ibex.h>
#include <math.h>

#include "vtk_graph.h"

using namespace std;
using namespace invariant;
using namespace ibex;

int main(int argc, char *argv[])
{
//    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/file_test.xml");
    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");

    IntervalVector search_space(3);

    array<array<size_t, 2>, 2> grid_limits; // X, Y limit data loading
    grid_limits[0][0] = 80; grid_limits[0][1] = 280;
    grid_limits[1][0] = 380; grid_limits[1][1] = 580;
//    grid_limits[0][0] = 0; grid_limits[0][1] = 584;
//    grid_limits[1][0] = 0; grid_limits[1][1] = 754;

    // ****** Dynamics *******
    double time_start_PM = omp_get_wtime();
    PreviMer3D pm3d = PreviMer3D(sources_xml, grid_limits);
    search_space = pm3d.get_search_space();
    cout << "TIME load PreviMer = " << omp_get_wtime() - time_start_PM << endl << endl;

/// **************** TEST 1 **************** ///
    IntervalVector test_position(3);
    test_position[0] = Interval(0, 14.0625);
    test_position[1] = Interval(52468.8, 52611.3);
    test_position[2] = Interval(99588.4, 99772.5);
    vector<ibex::IntervalVector> result = pm3d.eval(test_position);
    cout << "Result = " << endl;
    for(IntervalVector &iv:result)
        cout << iv << endl;

/// **************** TEST 2 **************** ///

//    Vtk_Graph monteCarlos("", NULL);
//    monteCarlos.monteCarlos(pm3d, 0, 130, 460);
//    monteCarlos(pm3d, 0, 131, 460);
//    monteCarlos(pm3d, 0, 131, 461);

#if 0
    // ****** Domain *******
    cout << "Search_space = " << search_space << endl;
    SmartSubPaving paving(search_space);
    invariant::Domain dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    const std::vector<double> limit_bisection = {15*60-1, 250-1, 250-1};
    paving.set_limit_bisection(limit_bisection);

    double t_c, x_c, y_c, r;
    t_c = 0 * pm3d.get_grid_conversion(0);
    x_c = 130 * pm3d.get_grid_conversion(1);
    y_c = 460 * pm3d.get_grid_conversion(2);
    r = 0.0;
    cout << "Center of initial set = " << t_c << " " << x_c << " " << y_c << endl;
    Variable t, x, y;
    Function f_sep(t, x, y, pow(t-t_c, 2)+pow(x-x_c, 2)+pow(y-y_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ******* Maze *********
    Maze maze(&dom, &pm3d);

    cout << "Domain = " << search_space << endl;

    double time_start = omp_get_wtime();
    for(int i=0; i<20; i++){
        cout << i << endl;
        double time_start_bisection = omp_get_wtime();
        paving.bisect();
        cout << " => bisection : " << omp_get_wtime() - time_start_bisection << "s - " << paving.size() << endl;
        maze.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << "s" << endl;

    cout << paving << endl;

    Vtk_Graph vtk_graph("Previmer", &paving, true);
//    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze);

    IntervalVector position(3);
    position[0] = Interval(t_c); // 450, 900
    position[1] = Interval(x_c); // 37304, 37980
    position[2] = Interval(y_c); // 119766, 120469
    vtk_graph.show_room_info(&maze, position);
#endif

    return 0;
}
