#ifndef DYNAMICS_FUNCTION_H
#define DYNAMICS_FUNCTION_H

#include "dynamics.h"
#include <ibex_IntervalVector.h>
#include <ibex_Function.h>
#include "omp.h"

namespace invariant {
class Dynamics_Function: public Dynamics
{
public:
    /**
     * @brief Dynamics constructor
     * @param functions
     */
    Dynamics_Function(const std::vector<ibex::Function *> functions, const DYNAMICS_SENS sens=FWD);

    /**
     * @brief Dynamics constructor
     * @param functions
     */
    Dynamics_Function(ibex::Function *functions, const DYNAMICS_SENS sens=FWD);

    /**
     * @brief Dynamics destructor
     */
    ~Dynamics_Function();

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
    const std::vector<ibex::IntervalVector> eval_d1(const ibex::IntervalVector &position);

    /**
     * @brief add_function_d1
     * @param functions_list
     */
    void add_function_d1(std::vector<ibex::Function *> &functions_list);

    /**
     * @brief add_function_d1
     * @param function
     */
    void add_function_d1(ibex::Function* f);

private:
    std::vector<ibex::Function*> m_functions;
    std::vector<ibex::Function*> m_functions_d1;
    omp_lock_t  m_lock_dynamics; // Lock
};
}
#endif // DYNAMICS_FUNCTION_H
