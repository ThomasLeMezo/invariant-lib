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

private:
    std::vector<ibex::Function*> m_functions;
    omp_lock_t  m_lock_dynamics; // Lock
};
}
#endif // DYNAMICS_FUNCTION_H
