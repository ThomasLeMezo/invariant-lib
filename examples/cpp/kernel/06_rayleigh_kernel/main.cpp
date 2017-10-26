#include "ibex/ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibes_graph.h"

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
    space[0] = Interval(-2,2);
    space[1] = Interval(-2,2);

    SmartSubPaving paving(space);

    double r = 0.5;

    // ****** Domain Outer ******* //
    invariant::Domain dom_outer(&paving, FULL_DOOR);

    Function f_sep_outer(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(r, 2));
    SepFwdBwd s_outer(f_sep_outer, GEQ); // LT, LEQ, EQ, GEQ, GT
    dom_outer.set_sep(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
    invariant::Domain dom_inner(&paving, FULL_WALL);

    Function f_sep_inner(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(r, 2));
    SepFwdBwd s_inner(f_sep_inner, LEQ); // LT, LEQ, EQ, GEQ, GT
    dom_inner.set_sep_input(&s_inner);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    // ****** Dynamics Outer ******* //
    ibex::Function f_outer(x1, x2, -Return(x2,
                                    -1.0*(x1+pow(x2,3)-x2)+Interval(-0.2, 0.2)));
    Dynamics_Function dyn_outer(&f_outer, BWD);

    // ****** Dynamics Inner ******* //
    ibex::Function f_inner1(x1, x2, Return(x2,
                                    -1.0*(x1+pow(x2,3)-x2)+Interval(-0.2)));
    ibex::Function f_inner2(x1, x2, Return(x2,
                                    -1.0*(x1+pow(x2,3)-x2)+Interval(0.2)));
    vector<Function *> f_list_inner;
    f_list_inner.push_back(&f_inner1);
    f_list_inner.push_back(&f_inner2);
    Dynamics_Function dyn_inner(f_list_inner, FWD);

    // ******* Mazes ********* //
    Maze maze_outer(&dom_outer, &dyn_outer);
    Maze maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    maze_inner.contract();
    for(int i=0; i<18; i++){
        paving.bisect();
        cout << i << " inner - " << maze_inner.contract() << " - " << paving.size() << endl;
        cout << i << " outer - " << maze_outer.contract() << " - " << paving.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    Vibes_Graph v_graph("SmartSubPaving", &maze_outer, &maze_inner);
    v_graph.setProperties(0, 0, 1024, 1024);
    v_graph.show();

//    Vibes_Graph v_graph2("graph_2", &maze_inner);
//    v_graph2.setProperties(0, 0, 1024, 1024);
//    v_graph2.show();

//    position_info[0] = Interval(1.745);
//    position_info[1] = Interval(-0.725);
//    v_graph2.get_room_info(&maze_inner, position_info);

    vibes::endDrawing();
}
