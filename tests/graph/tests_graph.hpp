#include "smartSubPaving.h"
#include "ibex/ibex_IntervalVector.h"

using namespace std;
using namespace ibex;
using namespace invariant;

invariant::SmartSubPaving<ibex::IntervalVector> graphTest_2D()
{
    IntervalVector coord(2);
    coord[0] = ibex::Interval(0, 1);
    coord[1] = ibex::Interval(3, 4);
    invariant::SmartSubPaving<ibex::IntervalVector> g(coord);
    return g;
}

invariant::SmartSubPaving<ibex::IntervalVector> graphTest_5D()
{
    IntervalVector coord(5);
    coord[0] = ibex::Interval(0, 1);
    coord[2] = ibex::Interval(3, 4);
    coord[3] = ibex::Interval(-1, -0.5);
    coord[4] = ibex::Interval(-100, 100);
    invariant::SmartSubPaving<ibex::IntervalVector> g(coord);
    return g;
}

invariant::SmartSubPaving<ibex::IntervalVector> graphTest_2D_Norm(){
    IntervalVector coord(2);
    coord[0] = ibex::Interval(-1,1);
    coord[1] = ibex::Interval(-1,1);
    invariant::SmartSubPaving<ibex::IntervalVector> g(coord);
    return g;
}

invariant::SmartSubPaving<ibex::IntervalVector> graphTest_5D_Norm(){
    IntervalVector coord(5);
    coord[0] = ibex::Interval(-1,1);
    coord[1] = ibex::Interval(-1,1);
    coord[2] = ibex::Interval(-1,1);
    coord[3] = ibex::Interval(-1,1);
    coord[4] = ibex::Interval(-1,1);
    invariant::SmartSubPaving<ibex::IntervalVector> g(coord);
    return g;
}
