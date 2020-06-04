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
  space[0] = ibex::Interval(-0.7, 0.7);
  space[1] = ibex::Interval(-1.2,1.2);

  ibex::Interval u(-1.0, 1.0);

  ibex::Function f_positive(x1, x2,-Return(x2,
                                          u));

  ibex::Function f_negative_1(x1, x2,-Return(x2,
                                          u.lb()+0.*x1));

  ibex::Function f_negative_2(x1, x2,-Return(x2,
                                          u.ub()+0.*x1));


  std::vector<ibex::Function*> f_list{&f_negative_1, &f_negative_2};

//  ibex::Function f(x1,x2,Return(x1,x2));
//  IntervalVector X(2);
//  X[0] = ibex::Interval(-0.7, 0.7);
//  X[1] = ibex::Interval(-1.2,1.2);
//  ibex::SepFwdBwd sep_id(f, X);

  largest_positive_invariant(space, &f_positive, f_list, 12, "kleene_ext2"/*, &sep_id*/);

//  largest_positive_invariant(space, &f_positive, 15, "dipole_positive");

//  largest_invariant(space, &f_positive, &f_negative, 15, "dipole_invariant");
}
