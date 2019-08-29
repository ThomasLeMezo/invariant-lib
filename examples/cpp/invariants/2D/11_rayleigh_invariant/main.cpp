#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"
#include "language.h"

#include "ibex_SepFwdBwd.h"

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
    space[0] = ibex::Interval(-2,2);
    space[1] = ibex::Interval(-2,2);


    Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(0.3, 2));


//    // ****** Dynamics ******* //
    ibex::Function f(x1, x2, -Return(x2,
                                    -1.0*(x1+pow(x2,3)-x2)));

    largest_positive_invariant(space, &f, 18, "rayleigh_positive_invariant");
}
