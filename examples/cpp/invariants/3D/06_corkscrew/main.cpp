#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>
#include "vtkmazeppl.h"
#include "language.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x(3);

    IntervalVector space(3);
    space[0] = ibex::Interval(-1, 1);
    space[1] = ibex::Interval(-1, 1);
    space[2] = ibex::Interval(-1, 1);

    ibex::Function f(x, Return(ibex::Interval(1),
                                     x[2],
                                     -x[1]-0.1*x[2]));
    invariant_PPL(space, &f, 15, "corkscrew", 15);

}
