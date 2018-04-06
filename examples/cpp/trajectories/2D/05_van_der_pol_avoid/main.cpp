#include "ibex_SepFwdBwd.h"

#include "eulerianmaze.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = ibex::Interval(-3,3);
    space[1] = ibex::Interval(-3,3);
    ibex::Function f(x1, x2, Return(x2,(1.0*(1.0-pow(x1, 2))*x2-x1)));

    // Sep A
    double xc_1, yc_1, r_1;
    xc_1 = 0.8; yc_1 = 1.3; r_1 = 0.4;
    Function f_sep_A(x1, x2, pow(x1-xc_1, 2)+pow(x2-yc_1, 2)-pow(r_1, 2));
    SepFwdBwd sep_A(f_sep_A, LEQ); // LT, LEQ, EQ, GEQ, GT)

    // Sep B
    IntervalVector box_B(2);
    box_B[0] = ibex::Interval(0.8, 1.45);
    box_B[1] = ibex::Interval(-0.2, 0);
    Function f_sep_B(x1, x2, Return(x1, x2));
    SepFwdBwd sep_B(f_sep_B, box_B);

    // Sep C
    IntervalVector box_C(2);
    box_C[0] = ibex::Interval(0.74, 1.2);
    box_C[1] = ibex::Interval(-1.5, -1.06);
    Function f_sep_C(x1, x2, Return(x1, x2));
    SepFwdBwd sep_C(f_sep_C, box_C);

    // Eulerian


    double time_start = omp_get_wtime();
    //    omp_set_num_threads(1);
    for(int i=0; i<18; i++){
        cout << i << endl;

    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;


    vibes::endDrawing();
}
