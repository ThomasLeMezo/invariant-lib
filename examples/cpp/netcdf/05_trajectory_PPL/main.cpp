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
//    PreviMer3D pm3d = PreviMer3D(sources_xml, grid_limits);
    PreviMer3D pm3d = PreviMer3D("PreviMer3D.data");

    search_space = pm3d.get_search_space();
    cout << "TIME load PreviMer = " << omp_get_wtime() - time_start_PM << endl << endl;
    cout << "Search_space = " << search_space << endl;

    // ****** Domain *******
    invariant::SmartSubPavingPPL paving(search_space);
    invariant::DomainPPL dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    const std::vector<double> limit_bisection = {15*60-1, 250-1, 250-1};
    paving.set_limit_bisection(limit_bisection);

    double t_c, x_c, y_c, r;
    t_c = 0 * pm3d.get_grid_conversion(0);
    x_c = 130 * pm3d.get_grid_conversion(1);
    y_c = 460 * pm3d.get_grid_conversion(2);
    r = 50.0;
    cout << "Center of initial set = " << t_c << " " << x_c << " " << y_c << endl;
    ibex::Variable t, x, y;
    Function f_sep(t, x, y, pow(t-t_c, 2)+pow(x-x_c, 2)+pow(y-y_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ******* Maze *********
    invariant::MazePPL maze(&dom, &pm3d);
    maze.set_widening_limit(20);

    cout << "Domain = " << search_space << endl;

    double time_start = omp_get_wtime();
    VtkMazePPL vtkMazePPL("PrevimerPPL");
//    omp_set_num_threads(1);

    for(int i=0; i<20; i++){
        cout << i << endl;
        paving.bisect();

        maze.get_domain()->set_init(FULL_WALL);
        maze.set_enable_contract_domain(true);
        maze.contract();

        maze.get_domain()->set_init(FULL_DOOR);
        maze.reset_nb_operations();
        maze.set_enable_contract_domain(false);
        maze.contract(30000);

        vtkMazePPL.show_maze(&maze);
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
//    vtkMazePPL.show_room_info(&maze, position);

    return 0;
}
