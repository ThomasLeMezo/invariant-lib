#include "graph.h"
#include "pave.h"
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
    space[0] = Interval(0,1);
    space[1] = Interval(0,1);
//    space[2] = Interval(0,1);

    IntervalVector intersect(2);
    intersect[0] = Interval(0.64, 0.75);
    intersect[1] = Interval(0.61, 0.8);

    Graph g(space);

    double t_start = omp_get_wtime();
    for(int i=0; i<20; i++){
        g.bisect();
    }
    cout << omp_get_wtime() - t_start << endl;
    vector<Pave *> list_p;
    vector<Face *> list_f;
    g.get_pave_node()->get_intersection_pave_outer(list_p, intersect);
    g.get_pave_node()->get_intersection_face_outer(list_f, intersect);
    cout << omp_get_wtime() - t_start << endl;

    cout << "list_f size = " << list_f.size() << endl;
    cout << "list_p size = " << list_p.size() << endl;

//    vibes::beginDrawing();
//    vibes::newFigure("Graph");
//    vibes::drawGraph(g, "b[]");
//    vibes::drawPave(g.get_paves());
//    vibes::drawPave(list_p, "b[b]");
//    vibes::drawFace(list_f, "r");

//    Vibes_Graph visu_g("graph " + to_string(i), &g);
//    visu_g.setProperties(0, 0, 512, 512);
//    visu_g.show();

//    Graphiz_Graph ggraph("graph.xdot", &g);
    cout << g << endl;
}
