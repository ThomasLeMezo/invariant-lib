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
  space[0] = ibex::Interval(-2, 2);
  space[1] = ibex::Interval(-2,2);

  ibex::Function f_positive(x1, x2, -Return((x1+1)/(pow(x1+1,2)+pow(x2,2)) - (x1-1)/(pow(x1-1,2)+pow(x2,2)),
                                  x2/(pow(x1+1,2)+pow(x2,2)) - x2/(pow(x1-1,2)+pow(x2,2))));

  ibex::Function f_negative(x1, x2, Return((x1+1)/(pow(x1+1,2)+pow(x2,2)) - (x1-1)/(pow(x1-1,2)+pow(x2,2)),
                                  x2/(pow(x1+1,2)+pow(x2,2)) - x2/(pow(x1-1,2)+pow(x2,2))));

//  largest_positive_invariant(space, &f_negative, 15, "dipole_negative");
//  largest_positive_invariant(space, &f_positive, 15, "dipole_positive");

  largest_invariant(space, &f_positive, &f_negative, 15, "dipole_invariant");
}
