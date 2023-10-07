#include "domain.h"

namespace invariant{
template<>
ppl::C_Polyhedron convert<ppl::C_Polyhedron>(const ibex::IntervalVector &iv){
    return iv_2_polyhedron(iv);
}

template<>
ibex::IntervalVector convert<ibex::IntervalVector>(const ibex::IntervalVector &iv){
    return iv;
}

template<>
ppl::C_Polyhedron convert<ppl::C_Polyhedron>(const ppl::C_Polyhedron &p){
    return p;
}

template<>
ibex::IntervalVector convert<ibex::IntervalVector>(const ppl::C_Polyhedron &p){
    return polyhedron_2_iv(p);
}

template<>
invariant::ExpBox convert<invariant::ExpBox>(const ibex::IntervalVector &iv){
    return ExpBox(iv);
}

template<>
ibex::IntervalVector convert<ibex::IntervalVector>(const invariant::ExpBox &q){
    return q.getBox();
}

template<>
invariant::ExpBox convert<invariant::ExpBox>(const invariant::ExpBox &q){
    return q;
}

}
