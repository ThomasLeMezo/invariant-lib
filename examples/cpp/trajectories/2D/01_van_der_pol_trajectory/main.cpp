#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibesMaze.h"

#include "booleantreeinter.h"
#include "booleantreeunion.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-4,4);
    space[1] = ibex::Interval(-4,4);
    invariant::SmartSubPaving<> paving(space);

    // ****** Dynamics *******
    ibex::Function f_A(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn_A(&f_A, FWD);

    ibex::Function f_B(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn_B(&f_B, BWD);

    // ****** Domain *******
    invariant::Domain<> dom_A(&paving, FULL_WALL);
    dom_A.set_border_path_in(false);
    dom_A.set_border_path_out(false);
    double xc_1, yc_1, r_1;
    xc_1 = -1.0;
    yc_1 = 1.0;
    r_1 = 0.5;
    Function f_sep_A(x1, x2, pow(x1-xc_1, 2)+pow(x2-yc_1, 2)-pow(r_1, 2));
    SepFwdBwd s_A(f_sep_A, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_A.set_sep_input(&s_A);

    invariant::Domain<> dom_B(&paving, FULL_WALL);
    dom_B.set_border_path_in(false);
    dom_B.set_border_path_out(false);
    IntervalVector box_B(2);
    box_B[0] = ibex::Interval(1.8, 2);
    box_B[1] = ibex::Interval(1.7, 2.4);
    Function f_sep_B(x1, x2, Return(x1, x2));
    SepFwdBwd s_B(f_sep_B, box_B);
    dom_B.set_sep_output(&s_B);

    // ****** Maze *******

    invariant::Maze<> mazeA(&dom_A, &dyn_A);
    invariant::Maze<> mazeB(&dom_B, &dyn_B);

    //    invariant::BooleanTreeUnionIBEX bisect_graph(&mazeA, &mazeB);
    invariant::BooleanTreeInterIBEX bisect_graph(&mazeA, &mazeB);
    paving.set_bisection_tree(&bisect_graph);

//    dom_B.add_maze_inter(&mazeA);
//    dom_A.add_maze_inter(&mazeB);
    dom_B.add_maze_inter_initial_condition(&mazeA);
    dom_A.add_maze_inter_initial_condition(&mazeB);

    // ****** Contractions *******
    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
        for(int j=0; j<2; j++){
            cout << "Maze A " << j << endl;
            mazeA.contract();

            cout << "Maze B " << j << endl;
            mazeB.contract();
        }
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    // ****** Visu *******

    VibesMaze v_mazeA("graph_A", &mazeA);
    v_mazeA.setProperties(0, 0, 512, 512);
    v_mazeA.show();
    v_mazeA.drawCircle(xc_1, yc_1, r_1, "r[]");
    v_mazeA.drawBox(box_B, "g[]");

    VibesMaze v_mazeB("graph_B", &mazeB);
    v_mazeB.setProperties(600, 0, 512, 512);
    v_mazeB.show();
    v_mazeB.drawBox(box_B, "r[]");
    v_mazeB.drawCircle(xc_1, yc_1, r_1, "g[]");

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-0.6);
    //    position_info[1] = ibex::Interval(0.6);
    //    v_mazeA.show_room_info(&mazeA, position_info);

    vibes::endDrawing();
}
