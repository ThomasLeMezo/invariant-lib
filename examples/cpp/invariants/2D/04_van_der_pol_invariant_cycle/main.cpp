#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);
    invariant::Domain<> dom(&paving, FULL_DOOR);

    Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(1.0, 2));
    SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, Return(x2,
                                    (1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn(&f, FWD_BWD, true);

    // ******* Maze ********* //
    invariant::Maze<> maze(&dom, &dyn);

    vibes::beginDrawing();

    vector<double> ptX, ptY;
    ptX.push_back(-1.5);
    ptY.push_back(0.814);
    double dt=0.01;
    for(double t=0; t<20; t+=dt){
        size_t k=ptX.size()-1;
        ptX.push_back(ptX[k]+dt*ptY[k]);
        ptY.push_back(ptY[k]+dt*((1-pow(ptX[k], 2))*ptY[k]-ptX[k]));
    }

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    omp_set_num_threads(1);
    for(int i=0; i<4; i++){
        cout << i << endl;
        paving.bisect();
//        if(i==7)
//            for(int j=184; j<185; j+=1){
//                maze.contract(j);

//                stringstream name;
//                name << "SmartSubPaving_" << j << "_";
//                VibesMaze v_maze(name.str(), &maze);
//                v_maze.setProperties(0, 0, 10, 10);
//                v_maze.show();
//                vibes::drawLine(ptX, ptY, "green[green]");

//                v_maze.saveImage("/home/lemezoth/Pictures/cycle_tests/", ".png");
//            }
//        else
            maze.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    VibesMaze v_maze("SmartSubPaving", &maze);
    v_maze.setProperties(0, 0, 512, 521);
    v_maze.show();

    IntervalVector position_info(2);
    position_info[0] = ibex::Interval(2);
    position_info[1] = ibex::Interval(0.3);
//    v_maze.show_room_info(&maze, position_info);

    vibes::endDrawing();

}
