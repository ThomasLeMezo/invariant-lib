#include "graph.h"

using namespace std;
using namespace ibex;
using namespace invariant;

Graph graphTest1()
{
    IntervalVector coord(2);
    coord[0] = Interval(0, 1);
    coord[1] = Interval(3, 4);
    Graph g(coord);
    return g;
}

Graph graphTest2()
{
    IntervalVector coord(5);
    coord[0] = Interval(0, 1);
    coord[2] = Interval(3, 4);
    coord[3] = Interval(-1, -0.5);
    coord[4] = Interval(-100, 100);
    Graph g(coord);
    return g;
}
