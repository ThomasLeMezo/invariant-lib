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
    string dir = string("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/data/");

    vector<double> grid_size = {15*60.0, 250.0, 250.0}; // {15*60 s, 250 m, 250 m}
    vector<double> limit_bisection = {60.0, 250.0/2.0, 250.0/2.0};

    IntervalVector search_space(3);
    search_space[0] = Interval(0, 8 * grid_size[0]); // T = 0..96 (in 15*min)
    search_space[1] = Interval(0, 300 * grid_size[1]); // X = 0..300
    search_space[2] = Interval(0, 300 * grid_size[2]); // Y = 200..500

    // ****** Dynamics *******
    double time_start_PM = omp_get_wtime();
    PreviMer3D pm3d = PreviMer3D(dir, search_space, grid_size, limit_bisection);
    cout << "TIME load PreviMer = " << omp_get_wtime() - time_start_PM << endl;

    // ****** Domain *******
    Graph graph(search_space);
    invariant::Domain dom(&graph, LINK_TO_INITIAL_CONDITION);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    graph.set_limit_bisection(limit_bisection);

    double t_c, x_c, y_c, r;
    t_c = 0;
    x_c = 150;
    y_c = 150;
    r = 0.5;
    Variable t, x, y;
    Function f_sep(t, x, y, pow(t-t_c, 2)+pow(x-x_c, 2)+pow(y-y_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ******* Maze *********
    Maze maze(&dom, &pm3d, MAZE_FWD, MAZE_PROPAGATOR);

    cout << "Domain = " << search_space << endl;

    double max_diam = search_space.max_diam();
    int iterations_max = 4*(ceil(log(max_diam)/log(2)));
    iterations_max = 15;

    double time_start = omp_get_wtime();
    maze.contract(); // To set first pave to be in
    for(int i=0; i<iterations_max; i++){
        graph.bisect();
        cout << i << "/" << iterations_max << " - " << maze.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vtk_Graph vtk_graph("Previmer", &graph, true);
    vtk_graph.show_graph();
    vtk_graph.show_maze(&maze);
}
