#include <iostream>
#include "previmer.h"
#include "maze.h"
#include <ibex.h>

#include "vibes_graph.h"

using namespace std;
using namespace invariant;
using namespace ibex;

int main(int argc, char *argv[])
{
    // ****** Dynamics *******
    double time_start_PM = omp_get_wtime();
    PreviMer pm = PreviMer("/home/lemezoth/Documents/ensta/flotteur/data/PREVIMER_L1-MARS2D-FINIS250_20081229T0000Z_MeteoMF.nc");
    cout << "TIME load PreviMer = " << omp_get_wtime() - time_start_PM << endl;

    // ****** Domain *******
//    IntervalVector search_space = pm.get_search_space();
    IntervalVector search_space(2);
    search_space[0] = Interval(0, 10);
    search_space[1] = Interval(0, 10);
    Graph graph(search_space);
    invariant::Domain dom(&graph);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 5;
    x2_c = 5;
    r = 0.1;
    Variable x1, x2;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ******* Maze *********
    Maze maze(&dom, &pm, MAZE_FWD, MAZE_PROPAGATOR);

    vibes::beginDrawing();
    Vibes_Graph v_graph("graph", &graph, &maze);
    v_graph.setProperties(0, 0, 512, 512);

    double time_start = omp_get_wtime();
    maze.contract(); // To set first pave to be in
    for(int i=0; i<8; i++){
        graph.bisect();
        cout << i << " - " << maze.contract() << " - " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    v_graph.show();
    vibes::endDrawing();

}
