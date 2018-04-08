#include "dynamics_function.h"

using namespace ibex;
using namespace std;
namespace invariant {

Dynamics_Function::Dynamics_Function(const vector<Function*> functions, const DYNAMICS_SENS sens, bool taylor):
    Dynamics(sens)
{
    m_functions = functions;
    compute_taylor(taylor);
    omp_init_lock(&m_lock_dynamics);
}

Dynamics_Function::Dynamics_Function(Function *f, const DYNAMICS_SENS sens, bool taylor):
    Dynamics(sens)
{
    m_functions.push_back(f);
    compute_taylor(taylor);
    omp_init_lock(&m_lock_dynamics);
}

Dynamics_Function::Dynamics_Function(Function *f1, Function *f2, const DYNAMICS_SENS sens, bool taylor):
    Dynamics(sens)
{
    m_functions.push_back(f1);
    m_functions.push_back(f2);
    compute_taylor(taylor);
    omp_init_lock(&m_lock_dynamics);
}

void Dynamics_Function::compute_taylor(bool taylor){
    if(taylor){
        for(Function* f:m_functions){
            ibex::Function *f_diff = new ibex::Function(*f, ibex::Function::DIFF);
            m_functions_d1.push_back(f_diff);
        }
    }
}

Dynamics_Function::~Dynamics_Function(){
    omp_destroy_lock(&m_lock_dynamics);
    for(Function* f:m_functions_d1)
        delete(f);
}

inline const std::vector<ibex::IntervalVector> Dynamics_Function::eval(const IntervalVector& position){
    vector<IntervalVector> vector_field;
    omp_set_lock(&m_lock_dynamics);
    for(Function*f:m_functions){
        ibex::IntervalVector result = f->eval_vector(position);
        vector_field.push_back(result);
    }
    omp_unset_lock(&m_lock_dynamics);
    return vector_field;
}

const std::vector<ibex::IntervalMatrix> Dynamics_Function::eval_d1(const ibex::IntervalVector &position){
    omp_set_lock(&m_lock_dynamics);
    vector<IntervalMatrix> vector_field;
    for(Function*f:m_functions_d1){
        IntervalMatrix jacobian= f->eval_matrix(position);
        vector_field.push_back(jacobian);
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

void Dynamics_Function::add_hybrid_condition(ibex::Sep *sep_guard, ibex::Function *f_reset_pos, ibex::Function *f_reset_neg){
    std::array<ibex::Function*, 2> tmp;
    tmp[0] = f_reset_pos;
    tmp[1] = f_reset_neg;
    m_hybrid_reset.insert(std::pair<ibex::Sep*, std::array<ibex::Function*, 2>>(sep_guard, tmp));
    m_hybrid_guard.push_back(sep_guard);
}


}
