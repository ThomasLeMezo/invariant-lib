#include "dynamics_function.h"

using namespace ibex;
using namespace std;
namespace invariant {

Dynamics_Function::Dynamics_Function(const vector<Function*> functions, const DYNAMICS_SENS sens):
Dynamics(sens)
{
    m_functions = functions;
    omp_init_lock(&m_lock_dynamics);
}

Dynamics_Function::Dynamics_Function(Function *functions, const DYNAMICS_SENS sens):
    Dynamics(sens)
{
    m_functions.push_back(functions);
    omp_init_lock(&m_lock_dynamics);
}

Dynamics_Function::~Dynamics_Function(){
    omp_destroy_lock(&m_lock_dynamics);
}

const std::vector<ibex::IntervalVector> Dynamics_Function::eval(const IntervalVector& position){
    omp_set_lock(&m_lock_dynamics);
    vector<IntervalVector> vector_field;
    for(Function*f:m_functions){
        IntervalVector result = f->eval_vector(position);
        vector_field.push_back(result);
    }
    omp_unset_lock(&m_lock_dynamics);
    return vector_field;
}

const std::vector<ibex::IntervalVector> Dynamics_Function::eval_d1(const ibex::IntervalVector &position){
    omp_set_lock(&m_lock_dynamics);
    vector<IntervalVector> vector_field;
    for(Function*f:m_functions_d1){
        IntervalMatrix jacobian= f->eval_matrix(position);
        IntervalVector result(position.size());
        for(int dim=0; dim<position.size(); dim++)
            result[dim] = jacobian[dim][dim];
        vector_field.push_back(result);
    }
    omp_unset_lock(&m_lock_dynamics);
    return vector_field;
}

void Dynamics_Function::add_function_d1(std::vector<Function *> &functions_list)
{
    for(Function* f:functions_list)
        m_functions_d1.push_back(f);
}

void Dynamics_Function::add_function_d1(Function *f)
{
    m_functions_d1.push_back(f);
}
}
