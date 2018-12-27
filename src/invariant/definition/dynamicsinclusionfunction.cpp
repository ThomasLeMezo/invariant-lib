#include "dynamicsinclusionfunction.h"

using namespace std;
using namespace ibex;

namespace invariant {
DynamicsInclusionFunction::DynamicsInclusionFunction(const std::vector<ibex::Function *> functions, const ibex::IntervalVector &inclusion_parameter, const DYNAMICS_SENS sens):
    DynamicsFunction(functions, sens), m_inclusion_parameter(inclusion_parameter)
{
}

DynamicsInclusionFunction::DynamicsInclusionFunction(ibex::Function *f,  const ibex::IntervalVector& inclusion_parameter, const DYNAMICS_SENS sens):
DynamicsFunction(f, sens), m_inclusion_parameter(inclusion_parameter)
{

}

const std::vector<ibex::IntervalVector> DynamicsInclusionFunction::eval(const ibex::IntervalVector &position){
    vector<IntervalVector> vector_field;
    IntervalVector val(position.size()+m_inclusion_parameter.size());
    for(int i=0; i<position.size(); i++)
        val[i] = position[i];
    for(int i=0; i<m_inclusion_parameter.size(); i++)
        val[i+position.size()] = m_inclusion_parameter[i];

    for(Function*f:m_functions[omp_get_thread_num()]){
        IntervalVector result = f->eval_vector(val);
        vector_field.push_back(result);
    }
    return vector_field;
}

//const std::vector<ibex::IntervalMatrix> DynamicsInclusionFunction::eval_d1(const ibex::IntervalVector &position){
//    return
//}

void DynamicsInclusionFunction::set_inclusion_parameter(const ibex::IntervalVector &parameter){
    m_inclusion_parameter = parameter;
}

void DynamicsInclusionFunction::set_inclusion_parameter(const ibex::Interval &parameter){
    m_inclusion_parameter[0] = parameter;
}

ibex::IntervalVector DynamicsInclusionFunction::get_inclusion_parameter(){
    return m_inclusion_parameter;
}
}
