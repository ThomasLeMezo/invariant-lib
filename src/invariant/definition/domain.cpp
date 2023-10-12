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
invariant::ExpPoly convert<invariant::ExpPoly>(const ibex::IntervalVector &iv){
    return ExpPoly(iv);
}

template<>
ibex::IntervalVector convert<ibex::IntervalVector>(const invariant::ExpPoly &q){
    return q.getBox();
}

template<>
invariant::ExpPoly convert<invariant::ExpPoly>(const invariant::ExpPoly &q){
    return q;
}

}
