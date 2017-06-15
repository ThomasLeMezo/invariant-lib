#include "graph.h"
#include "pave.h"
#include "VibesFigure_Graph.h"
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
    space[0] = Interval(0,1);
    space[1] = Interval(0,1);
//    space[2] = Interval(0,1);

    Graph g(space);
//    cout << g << endl;
    double t_start = omp_get_wtime();
    for(int i=0; i<10; i++){
        g.bisect();
    }

    IntervalVector intersect(2);
    intersect[0] = Interval(0.6, 0.7);
    intersect[1] = Interval(0.6, 0.7);

    vector<Pave *> list_p;
    g.get_pave_node()->get_intersection_pave_outer(list_p, intersect);
    cout << list_p << endl;

    cout << omp_get_wtime() - t_start << endl;

    //        VibesFigure_Graph visu_g("graph " + to_string(i), &g);
    //        visu_g.setProperties(0, 0, 512, 512);
    //        visu_g.show();

    //Graphiz_Graph ggraph("graph.xdot", &g);
    cout << g << endl;
}
