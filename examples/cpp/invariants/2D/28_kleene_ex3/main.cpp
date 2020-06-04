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
  space[0] = ibex::Interval(-1.2, 1.2);
  space[1] = ibex::Interval(-1.2, 1.2);

//  ibex::Interval u(-1, 1);

  ibex::Function f_positive(x1, x2,-Return(-2*x2,
                                          0.8*x1+10*(pow(1.02,2)*x1*x1-0.2)*x2));

  ibex::Function f_negative(x1, x2,-Return(-2*x2,
                                           0.8*x1+10*(pow(1.02,2)*x1*x1-0.2)*x2));

  std::vector<ibex::Function*> f_list{&f_negative};

  ibex::Function f(x1,x2,x1*x1+x2*x2-1);
  ibex::SepFwdBwd sep_id(f, ibex::LEQ);

  largest_positive_invariant(space, &f_positive, f_list, 16, "kleene_ext3", &sep_id);

//  largest_positive_invariant(space, &f_positive, 15, "dipole_positive");

//  largest_invariant(space, &f_positive, &f_negative, 15, "dipole_invariant");
}
