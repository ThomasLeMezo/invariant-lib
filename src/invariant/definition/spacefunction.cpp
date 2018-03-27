#include "spacefunction.h"
#include "iostream"

using namespace std;
using namespace ibex;

namespace invariant {

SpaceFunction::SpaceFunction(){
}

ibex::IntervalVector SpaceFunction::eval_vector(const ibex::IntervalVector &position) const{
    ibex::IntervalVector vect(position.size(), ibex::Interval::EMPTY_SET);
    for(size_t i=0; i<m_functions_list.size(); i++){
        // Evaluate the separator
        ibex::Sep *sep = m_separator_list[i];

        ibex::IntervalVector x_in(position);
        ibex::IntervalVector x_out(x_in);

        sep->separate(x_in, x_out);

        if(x_in.is_empty() || !x_out.is_empty()){ // There exist point inside the constraint
            // Evaluate the function
            ibex::Function *f = m_functions_list[i];
            vect |= f->eval_vector(position);
        }
    }
    return vect;
}

void SpaceFunction::push_back(ibex::Function *f, ibex::Sep *sep){
    m_functions_list.push_back(f);
    m_separator_list.push_back(sep);
}

}
