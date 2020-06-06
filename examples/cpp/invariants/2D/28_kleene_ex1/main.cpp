#include "ibex_Function.h"
#include "ibex_SepFwdBwd.h"
#include "ibex_IntervalVector.h"
#include <iostream>
#include "language.h"

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[]){
  ibex::Variable x1, x2;

  IntervalVector space(2);
  space[0] = ibex::Interval(-1, 1);
  space[1] = ibex::Interval(-1.3,1.3);

  ibex::Function f_positive(x1, x2,Return(0.5*x1+x1*x1-x2*x2,
                                          -0.5*x2+x1*x1));

  ibex::Function f_negative(x1, x2,-Return(0.5*x1+x1*x1-x2*x2,
                                          -0.5*x2+x1*x1));

  largest_positive_invariant(space, &f_negative, 15, "kleene_ext1");
//  largest_positive_invariant(space, &f_positive, 15, "dipole_positive");

//  largest_invariant(space, &f_positive, &f_negative, 15, "dipole_invariant");
}
