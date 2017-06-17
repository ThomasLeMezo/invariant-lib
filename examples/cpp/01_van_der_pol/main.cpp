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
    IntervalVector space(2);
    space[0] = Interval(-6,6);
    space[1] = Interval(-6,6);

    // ****** Domain *******
    Graph graph(space);
    invariant::Domain dom(&graph);

    Function f_sep("x1", "x2", "x1^2+x2^2-2");
    SepFwdBwd s(f_sep, LT);
    dom.set_sep(&s);

    // ****** Dynamics *******
    ibex::Variable x1, x2;
    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));
    Dynamics_Function dyn(&f);

    // ******* Maze *********
    Maze maze(&dom, &dyn);

    for(int i=0; i<10; i++){
        graph.bisect();
        cout << maze.contract() << " - ";
        cout << graph.size() << endl;
    }

//    for(Pave*p:graph.get_paves()){
//        cout << *p << endl;
//    }

    Vibes_Graph v_graph("graph", &graph);
    v_graph.show();
    cout << graph << endl;
}
