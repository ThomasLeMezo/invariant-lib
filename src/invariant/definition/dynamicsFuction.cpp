#include "dynamicsFunction.h"
#include "omp.h"

using namespace ibex;
using namespace std;
namespace invariant {

DynamicsFunction::DynamicsFunction(const vector<Function*> functions, const DYNAMICS_SENS sens, bool taylor):
    Dynamics(sens)
{
    init(functions, sens, taylor);
}

void DynamicsFunction::init(const vector<Function*> functions, const DYNAMICS_SENS sens, bool taylor){
    m_num_threads = omp_get_max_threads();
    for(int n = 0; n<m_num_threads; n++){
        m_functions.push_back(vector<Function*>());
        for(Function* f:functions){
            Function *f_new = new Function(*f, Function::COPY);
            m_functions[n].push_back(f_new);
        }
    }

    compute_taylor(taylor);
//    omp_init_lock(&m_lock_dynamics);
}

DynamicsFunction::DynamicsFunction(Function *f, const DYNAMICS_SENS sens, bool taylor):
    Dynamics(sens)
{
    vector<Function*> functions;
    functions.push_back(f);
    init(functions, sens, taylor);
}

DynamicsFunction::DynamicsFunction(Function *f1, Function *f2, const DYNAMICS_SENS sens, bool taylor):
    Dynamics(sens)
{
    vector<Function*> functions;
    functions.push_back(f1);
    functions.push_back(f2);
    init(functions, sens, taylor);
}

void DynamicsFunction::compute_taylor(bool taylor){
    if(taylor){
        for(int n = 0; n<m_num_threads; n++){
            m_functions_d1.push_back(vector<Function*>());
            for(Function* f:m_functions[n]){
                ibex::Function *f_diff = new ibex::Function(*f, ibex::Function::DIFF);
                m_functions_d1[n].push_back(f_diff);
            }
        }
    }
}

DynamicsFunction::~DynamicsFunction(){
//    omp_destroy_lock(&m_lock_dynamics);
    for(int n=0; n<m_num_threads; n++){
        if(m_functions_d1.size()>(size_t)n){
            for(Function* f:m_functions_d1[n])
                delete(f);
        }
        if(m_functions.size()>(size_t)n){
            for(Function* f:m_functions[n])
                delete(f);
        }
    }
}

inline const std::vector<ibex::IntervalVector> DynamicsFunction::eval(const IntervalVector& position){
    vector<IntervalVector> vector_field;
//    omp_set_lock(&m_lock_dynamics);
    for(Function*f:m_functions[omp_get_thread_num()]){
        ibex::IntervalVector result = f->eval_vector(position);
        vector_field.push_back(result);
    }
//    omp_unset_lock(&m_lock_dynamics);
    return vector_field;
}

const std::vector<ibex::IntervalMatrix> DynamicsFunction::eval_d1(const ibex::IntervalVector &position){
//    omp_set_lock(&m_lock_dynamics);
    vector<IntervalMatrix> vector_field;
    if(!m_functions_d1.empty()){
        for(Function*f:m_functions_d1[omp_get_thread_num()]){
            IntervalMatrix jacobian= f->eval_matrix(position);
            vector_field.push_back(jacobian);
        }
    }
//    omp_unset_lock(&m_lock_dynamics);
    return vector_field;
}

void DynamicsFunction::add_hybrid_condition(ibex::Sep *sep_guard, ibex::Function *f_reset_pos, ibex::Function *f_reset_neg){
    std::array<ibex::Function*, 2> tmp;
    tmp[0] = f_reset_pos;
    tmp[1] = f_reset_neg;
    m_hybrid_reset.insert(std::pair<ibex::Sep*, std::array<ibex::Function*, 2>>(sep_guard, tmp));
    m_hybrid_guard.push_back(sep_guard);
}


}
