#ifndef DynamicsFunction_H
#define DynamicsFunction_H

#include "dynamics.h"

#include <ibex_IntervalVector.h>
#include <ibex_Function.h>
#include <ibex_Sep.h>

#include <map>

#include <omp.h>

namespace invariant {
class DynamicsFunction: public Dynamics
{
public:
    /**
     * @brief Dynamics constructor
     * @param functions
     */
    DynamicsFunction(const std::vector<ibex::Function *> functions, const DYNAMICS_SENS sens=FWD);

    /**
     * @brief Dynamics constructor
     * @param functions
     */
    DynamicsFunction(ibex::Function *f, const DYNAMICS_SENS sens=FWD);
    DynamicsFunction(ibex::Function *f1, ibex::Function *f2, const DYNAMICS_SENS sens=FWD);

    /**
     * @brief Dynamics destructor
     */
    ~DynamicsFunction();

    /**
     * @brief init
     * @param functions
     * @param sens
     * @param taylor
     */
    void init(const std::vector<ibex::Function*> functions);

    /**
     * @brief Eval the vector field from the state space
     * @param position
     * @return
     */
    const std::vector<ibex::IntervalVector> eval(const ibex::IntervalVector &position);

    /**
     * @brief Eval the derivate vector field from the state space
     * @param position
     * @return
     */
    const std::vector<ibex::IntervalMatrix> eval_d1(const ibex::IntervalVector &position);

    /**
     * @brief add_hybrid_condition
     * @param f_guard
     * @param f_reset
     */
    void add_hybrid_condition(ibex::Sep *sep_guard, ibex::Function *f_reset_pos, ibex::Function* f_reset_neg);

    /**
     * @brief get_hybrid_guard
     * @return
     */
    std::vector<ibex::Sep*> get_hybrid_guard() override;

    /**
     * @brief get_hybrid_reset
     * @return
     */
    std::map<ibex::Sep*, std::array<ibex::Function*, 2>> get_hybrid_reset() override;

private:
//    /**
//     * @brief add_function_d1
//     * @param functions_list
//     */
//    void add_function_d1(std::vector<ibex::Function *> &functions_list);

//    /**
//     * @brief add_function_d1
//     * @param function
//     */
//    void add_function_d1(ibex::Function* f);

    /**
     * @brief compute taylor functions
     */
    void compute_taylor(bool taylor);

protected:
    std::vector<std::vector<ibex::Function*>> m_functions;
    std::vector<std::vector<ibex::Function*>> m_functions_d1;

    std::vector<ibex::Sep*> m_hybrid_guard;
    std::map<ibex::Sep*, std::array<ibex::Function*, 2>> m_hybrid_reset;

    omp_lock_t  m_lock_dynamics; // Lock

    int m_num_threads = 1;
};

inline std::vector<ibex::Sep*> DynamicsFunction::get_hybrid_guard(){
    return m_hybrid_guard;
}

inline std::map<ibex::Sep*, std::array<ibex::Function*, 2>> DynamicsFunction::get_hybrid_reset(){
    return m_hybrid_reset;
}

}
#endif // DynamicsFunction_H
