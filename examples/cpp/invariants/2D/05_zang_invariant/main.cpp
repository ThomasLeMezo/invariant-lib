#include "ibex_Function.h"
#include "ibex_SepFwdBwd.h"
#include "ibex_IntervalVector.h"
#include <iostream>
#include "language.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
  ibex::Variable x1, x2;

  IntervalVector space(2);
  space[0] = ibex::Interval(-2.5, 2.5);
  space[1] = ibex::Interval(-2,2);

  Function f_sep(x1, x2, pow(x1, 2)+pow(x2, 2)-pow(0.5, 2));
  SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT

  ibex::Function f(x1, x2, -Return(x2,
                                  (8.0/25.0*pow(x1,5)-4.0/3.0*pow(x1,3)+4.0/5.0*x1)));

  largest_positive_invariant(space, &f, 15, "zang_positive_invariant", &s);
}
