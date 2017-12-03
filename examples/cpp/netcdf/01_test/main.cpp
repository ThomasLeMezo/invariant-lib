#include "ibex/ibex_SepFwdBwd.h"
#include <iostream>

#include "smartSubPaving.h"
#include "domain.h"
#include "maze.h"
#include "previmer.h"

#include <ibex.h>
#include <math.h>

#include "vibesMaze.h"

using namespace std;
using namespace invariant;
using namespace ibex;

int main(int argc, char *argv[])
{
    // ****** Dynamics *******
    double time_start_PM = omp_get_wtime();
//    PreviMer pm = PreviMer("/home/lemezoth/Documents/ensta/flotteurs/data_ifremer/data/PREVIMER_L1-MARS2D-FINIS250_20161219T0000Z_MeteoMF.nc");
    PreviMer pm = PreviMer("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/data/MARC_L1-MARS2D-FINIS250_20170709T0000Z_MeteoMF.nc");
    cout << "TIME load PreviMer = " << omp_get_wtime() - time_start_PM << endl;

    // ****** Domain *******
    IntervalVector search_space = pm.get_search_space();
//    IntervalVector search_space(2);
//    search_space[0] = ibex::Interval(0, 300);
//    search_space[1] = ibex::Interval(200, 500);
    invariant::SmartSubPaving<> paving(search_space);
    invariant::Domain<> dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    double x1_c, x2_c, r;
    x1_c = 210;
    x2_c = 400;
    r = 0.0;
    ibex::Variable x1, x2;
    Function f_sep(x1, x2, pow(x1-x1_c, 2)+pow(x2-x2_c, 2)-pow(r, 2));
    SepFwdBwd s(f_sep, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom.set_sep(&s);

    // ******* Maze *********
    invariant::Maze<> maze(&dom, &pm);

    double max_diam = max(search_space[0].diam(), search_space[1].diam());
    int iterations_max = 2*(ceil(log(max_diam)/log(2)));

    double time_start = omp_get_wtime();
    maze.contract(); // To set first pave to be in
    for(int i=0; i<iterations_max+1; i++){
        paving.bisect();
        cout << i << " - " << maze.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("SmartSubPaving", &maze);
    v_maze.setProperties(0, 0, 512, 512);
    v_maze.show();

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(x1_c);
    position_info[1] = ibex::Interval(x2_c);
    v_maze.get_room_info(&maze, position_info);


    vibes::endDrawing();


    /// DEBUG
    short fill_value = pm.get_fill_value();

    vibes::beginDrawing();
    vibes::newFigure("test");
    for(size_t i=0; i<pm.get_i_max(); i+=2){
        for(size_t j=0; j<pm.get_j_max(); j+=2){
            if(pm.get_raw_u()[i][j]!=fill_value){
                vibes::drawBox(i, i+1, j, j+1, "b[b]");
            }
            else{
                vibes::drawBox(i, i+1, j, j+1, "grey[grey]");
            }
        }
    }
    vibes::axisAuto("test");
}
