#include "graph.h"
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
    space[0] = Interval(-6,6);
    space[1] = Interval(-6,6);

    // ****** Domain *******
    Graph graph(space);

    invariant::Domain dom(&graph);

    // Select only the border of the graph (ToDo : improve)
    Interval size = Interval(ibex::previous_float(6));
    Function f_sep0(x1, x2, x1-size); SepFwdBwd s1(f_sep0, LEQ); // LT, LEQ, EQ, GEQ, GT
    Function f_sep1(x1, x2, x1+size); SepFwdBwd s2(f_sep1, GEQ);
    Function f_sep2(x1, x2, x2-size); SepFwdBwd s3(f_sep2, LEQ);
    Function f_sep3(x1, x2, x2+size); SepFwdBwd s4(f_sep3, GEQ);
    Array<Sep> s_a = {s1, s2, s3, s4};
    SepInter s_u(s_a);
    dom.set_sep_input(&s_u);

    // ****** Dynamics *******
    ibex::Function f1(x1, x2, Return(-x2,-(1.0*(1.0-pow(x1, 2))*x2-x1)+0.5));
    ibex::Function f2(x1, x2, Return(-x2,-(1.0*(1.0-pow(x1, 2))*x2-x1)-0.5));
    vector<Function*> f_list;
    f_list.push_back(&f1);
//    f_list.push_back(&f2);
    Dynamics_Function dyn(f_list);

    // ******* Maze *********
    Maze maze(&dom, &dyn);

    double time_start = omp_get_wtime();
    for(int i=0; i<15; i++){
        graph.bisect();
        cout << i << " | " << maze.contract() << " | " << graph.size() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vibes_Graph v_graph("graph", &graph, &maze);
    v_graph.setProperties(0, 0, 512, 512);
    v_graph.show();

    v_graph.get_room_info(&maze, 4, 4);

}
