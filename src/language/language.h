#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "ibex_IntervalVector.h"
#include "ibex_Function.h"
#include <fstream>
#include <string>

namespace invariant {
 int largest_positive_invariant(ibex::IntervalVector &space,
                                ibex::Function *f_dyn, size_t nb_steps=0, std::string file_name="positiveInvariant", ibex::Function* f_dom=nullptr);
}

#endif // LANGUAGE_H
