#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "ibex_IntervalVector.h"
#include "ibex_Function.h"
#include "ibex_Sep.h"
#include <fstream>
#include <string>
#include <vector>

namespace invariant {
 int largest_positive_invariant(ibex::IntervalVector &space,
                                ibex::Function *f_dyn,
                                size_t nb_steps,
                                std::string file_name,
                                ibex::Function* f_dom
                                );

 int largest_positive_invariant(ibex::IntervalVector &space,
                                ibex::Function *f_dyn,
                                size_t nb_steps,
                                std::string file_name
                                );

 int largest_positive_invariant(ibex::IntervalVector &space,
                                ibex::Function *f_dyn,
                                size_t nb_steps,
                                std::string file_name,
                                ibex::Sep* sep_outer
                                );

 int largest_positive_invariant(ibex::IntervalVector &space,
                                ibex::Function* f_outer,
                                std::vector<ibex::Function*> &f_inner,
                                size_t nb_steps,
                                std::string file_name,
                                ibex::Sep* sep_outer
                                );

 int largest_positive_invariant_python(ibex::IntervalVector &space,
                                       ibex::Function* f_outer,
                                       size_t nb_steps,
                                       std::string file_name,
                                       std::vector<ibex::Function*> &f_inner,
                                       ibex::Sep* sep_outer);

 int invariant_PPL(ibex::IntervalVector &space,
                   ibex::Function *f_dyn,
                   size_t nb_steps,
                   std::string file_name,
                   size_t contraction_limit=15);

 int largest_invariant(ibex::IntervalVector &space,
                       ibex::Function *f_dyn_positive,
                       ibex::Function *f_dyn_negative,
                       size_t nb_steps,
                       std::string file_name);
}

#endif // LANGUAGE_H
