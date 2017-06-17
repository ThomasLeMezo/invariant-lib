#include "dynamics_function.h"

using namespace ibex;
using namespace std;
namespace invariant {

Dynamics_Function::Dynamics_Function(const vector<Function*> functions){
    m_functions = functions;
}

Dynamics_Function::Dynamics_Function(Function *functions){
    m_functions.push_back(functions);
}

const std::vector<ibex::IntervalVector> Dynamics_Function::eval(IntervalVector position){
    vector<IntervalVector> vector_field;
    for(Function*f:m_functions){
        IntervalVector result = f->eval_vector(position);
        vector_field.push_back(result);
    }
    return vector_field;
}
}
