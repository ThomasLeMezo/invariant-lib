#include "dynamicsFunction.h"
#include "omp.h"

using namespace ibex;
using namespace std;
namespace invariant {

DynamicsFunction::DynamicsFunction(const vector<Function*> &functions, const DYNAMICS_SENS sens, bool multi_threaded):
    Dynamics(sens)
{
    m_multi_threaded = multi_threaded;
    initalize_function(functions);

}

void DynamicsFunction::initalize_function(const vector<Function*> &functions){
    if(m_multi_threaded){
        m_num_threads = omp_get_max_threads();
        cout << "Maximum nb thread = " << m_num_threads << endl;
        //    cout << "[DynamicsFunction] " << m_num_threads << " threads" << endl;
        for(int n = 0; n<m_num_threads; n++){
            m_functions.push_back(vector<Function*>());
            for(Function* f:functions){
                Function *f_new;
                if(dynamic_cast<SpaceFunction*>(f) == nullptr)
                    f_new = new Function(*f);
                else
                    f_new = new SpaceFunction(*dynamic_cast<SpaceFunction*>(f));
                m_functions[n].push_back(f_new);
            }
        }
        compute_taylor(true);
    }
    else{
        m_functions.push_back(functions);
        compute_taylor(true);
//        m_functions.push_back(vector<Function*>());
//        for(Function* f:functions){

//        }
        omp_init_lock(&m_lock_dynamics);
    }
}

//DynamicsFunction::DynamicsFunction(const vector<SpaceFunction *> functions, const DYNAMICS_SENS sens, bool multi_threaded):
//Dynamics(sens)
//{

//}

DynamicsFunction::DynamicsFunction(Function *f, const DYNAMICS_SENS sens, bool multi_threaded):
    Dynamics(sens)
{
    m_multi_threaded = multi_threaded;
    vector<Function*> functions{f};
    initalize_function(functions);
}

DynamicsFunction::DynamicsFunction(Function *f1, Function *f2, const DYNAMICS_SENS sens, bool multi_threaded):
    Dynamics(sens)
{
    m_multi_threaded = multi_threaded;
    vector<Function*> functions;
    functions.push_back(f1);
    functions.push_back(f2);
    initalize_function(functions);
}

DynamicsFunction::DynamicsFunction(SpaceFunction *f, const DYNAMICS_SENS sens, bool multi_threaded):
    Dynamics(sens)
{
    m_multi_threaded = multi_threaded;
    vector<Function*> functions{f};
    initalize_function(functions);
}

//DynamicsFunction::DynamicsFunction(SpaceFunction *f1, Function *f2, const DYNAMICS_SENS sens, bool multi_threaded):
//    Dynamics(sens)
//{
//    m_multi_threaded = multi_threaded;
//    vector<Function*> functions;
//    functions.push_back(f1);
//    functions.push_back(f2);
//    initalize_function(functions);
//}

void DynamicsFunction::compute_taylor(bool taylor){
    if(taylor){
        for(int n = 0; n<m_functions.size(); n++){
            m_functions_d1.push_back(vector<Function*>());
            for(Function* f:m_functions[n]){
                ibex::Function *f_diff = new ibex::Function(*f, ibex::Function::DIFF);
                m_functions_d1[n].push_back(f_diff);
            }
        }
    }
}

DynamicsFunction::~DynamicsFunction(){
    if(m_multi_threaded){
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
    else{
        omp_destroy_lock(&m_lock_dynamics);
    }
}

const std::vector<ibex::IntervalVector> DynamicsFunction::eval(const IntervalVector& position){
    vector<IntervalVector> vector_field;
    size_t thread_id = 0;
    if(m_multi_threaded)
        thread_id = omp_get_thread_num();
    else
        omp_set_lock(&m_lock_dynamics);

    for(Function*f:m_functions[thread_id]){
        if(dynamic_cast<SpaceFunction*>(f)==nullptr){
            ibex::IntervalVector result = f->eval_vector(position);
            vector_field.push_back(result);
        }
        else{
            ibex::IntervalVector result = dynamic_cast<SpaceFunction*>(f)->eval_vector(position);
            vector_field.push_back(result);
        }
    }

    if(!m_multi_threaded)
        omp_unset_lock(&m_lock_dynamics);
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

const std::vector<std::pair<ibex::Matrix,ibex::IntervalVector>> DynamicsFunction::eval_jac(const ibex::IntervalVector &position){
    //    omp_set_lock(&m_lock_dynamics);
    vector<std::pair<Matrix,IntervalVector>> vector_field;
    size_t thread_id = 0;
    if(m_multi_threaded)
        thread_id = omp_get_thread_num();
    else
        omp_set_lock(&m_lock_dynamics);

    if(!m_functions_d1.empty()){
        int n = m_functions[thread_id].size();
        assert(m_functions_d1[thread_id].size()==n);
        for (int i=0;i<n;i++) {
            Function*f_d1 = m_functions_d1[thread_id][i];
            Function*f    = m_functions[thread_id][i];
            IntervalMatrix jacobian= f_d1->eval_matrix(position);
            Matrix jcenter = jacobian.mid();
            jacobian = jacobian-jcenter;
            IntervalVector fmid = f->eval_vector(position.mid());
            fmid += jacobian * (position - position.mid());
            vector_field.push_back(pair<Matrix,IntervalVector>
					(jcenter,fmid));
        }
    }

    if(!m_multi_threaded)
        omp_unset_lock(&m_lock_dynamics);
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
