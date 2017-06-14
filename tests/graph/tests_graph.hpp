#include "graph.h"

using namespace std;
using namespace ibex;
using namespace invariant;

Graph graphTest_2D()
{
    IntervalVector coord(2);
    coord[0] = Interval(0, 1);
    coord[1] = Interval(3, 4);
    Graph g(coord);
    return g;
}

Graph graphTest_5D()
{
    IntervalVector coord(5);
    coord[0] = Interval(0, 1);
    coord[2] = Interval(3, 4);
    coord[3] = Interval(-1, -0.5);
    coord[4] = Interval(-100, 100);
    Graph g(coord);
    return g;
}

Graph graphTest_2D_Norm(){
    IntervalVector coord(2);
    coord[0] = Interval(-1,1);
    coord[1] = Interval(-1,1);
    Graph g(coord);
    return g;
}

Graph graphTest_5D_Norm(){
    IntervalVector coord(5);
    coord[0] = Interval(-1,1);
    coord[1] = Interval(-1,1);
    coord[2] = Interval(-1,1);
    coord[3] = Interval(-1,1);
    coord[4] = Interval(-1,1);
    Graph g(coord);
    return g;
}
