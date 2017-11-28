#include "room.h"
#include "ibex/ibex_IntervalVector.h"

namespace invariant{

/// ******************  ibex::IntervalVector ****************** ///

template<>
void Room<ibex::IntervalVector>::contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect){
    // assert 0 not in v.
    ibex::IntervalVector c(out-in);
    ibex::IntervalVector v(vect);
    ibex::Interval alpha(ibex::Interval::POS_REALS);

    for(int i=0; i<v.size(); i++){
        if(!(c[i]==ibex::Interval::ZERO && ibex::Interval::ZERO.is_subset(v[i])))
            alpha &= ((c[i]/(v[i] & ibex::Interval::POS_REALS)) & ibex::Interval::POS_REALS) | ((c[i]/(v[i] & ibex::Interval::NEG_REALS)) & ibex::Interval::POS_REALS);
    }
    if(alpha==ibex::Interval::ZERO)
        alpha.set_empty();

    c &= alpha*v;
    out &= c+in;
    in &= out-c;
}

/// ******************  ppl::C_Polyhedron ****************** ///


/// ******************  Other functions ****************** ///

int get_nb_dim_flat(const ibex::IntervalVector &iv){
    int dim = iv.size();
    int flat=0;
    for(int i=0; i<dim; i++){
        if(iv[i].is_degenerated() && !iv[i].is_unbounded())
            flat++;
    }
    return flat;
}

}
