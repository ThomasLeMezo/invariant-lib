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
    // ****** Dynamics *******
//    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/file_test.xml");
    string sources_xml = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");

    array<array<size_t, 2>, 2> grid_limits; // X, Y limit data loading
    grid_limits[0][0] = 80; grid_limits[0][1] = 280; // max = 584
    grid_limits[1][0] = 380; grid_limits[1][1] = 580; // max = 754

    double time_start_PM = omp_get_wtime();
    PreviMer3D pm3d = PreviMer3D(sources_xml, grid_limits);
//    PreviMer3D pm3d = PreviMer3D("PreviMer3D.data");

    // ****** Domain *******
    IntervalVector search_space(3);
    search_space = pm3d.get_search_space();
    cout << "TIME load PreviMer = " << omp_get_wtime() - time_start_PM << endl << endl;
    cout << "Search_space = " << search_space << endl;

    invariant::SmartSubPavingPPL paving(search_space);
    invariant::DomainPPL dom(&paving, FULL_WALL);

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
    dom.set_sep_input(&s);

    // ******* Maze *********
    invariant::MazePPL maze(&dom, &pm3d);
    cout << "Domain = " << search_space << endl;

    double time_start = omp_get_wtime();
    VtkMazePPL vtkMazePPL("PrevimerPPL");
//    omp_set_num_threads(1);

    maze.set_widening_limit(15);
    maze.set_enable_contraction_limit(true);
    maze.set_contraction_limit(8);
//    int factor_door = 3;

    paving.set_enable_bisection_strategy(0, true);
    paving.set_bisection_strategy_slice(0, 900);

    for(int i=0; i<17; i++){
        std::time_t t_now = std::time(nullptr);
        cout << i << " - " << std::ctime(&t_now);
        paving.bisect();

        maze.get_domain()->set_init(FULL_WALL);
        maze.set_enable_contract_domain(true);
        maze.contract();

//        maze.get_domain()->set_init(FULL_DOOR);
//        maze.reset_nb_operations();
//        maze.set_enable_contract_domain(false);
//        maze.contract(paving.size_active()*factor_door);

        vtkMazePPL.show_maze(&maze);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << "s" << endl;
    cout << paving << endl;

    IntervalVector position(3);
    position[0] = ibex::Interval(5350); // 450, 900
    position[1] = ibex::Interval(34200); // 37304, 37980
    position[2] = ibex::Interval(126600); // 119766, 120469
    vtkMazePPL.show_room_info(&maze, position);

    return 0;
}
