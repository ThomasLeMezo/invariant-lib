#ifndef DYNAMICS_FUNCTION_H
#define DYNAMICS_FUNCTION_H

#include "dynamics.h"
#include <ibex.h>
#include "omp.h"

namespace invariant {
class Dynamics_Function: public Dynamics
{
public:
    /**
     * @brief Dynamics constructor
     * @param functions
     */
    Dynamics_Function(const std::vector<ibex::Function *> functions);

    /**
     * @brief Dynamics constructor
     * @param functions
     */
    Dynamics_Function(ibex::Function *functions);

    /**
     * @brief Dynamics destructor
     */
    ~Dynamics_Function();

    /**
     * @brief Eval the vector field from the state space
     * @param position
     * @return
     */
    const std::vector<ibex::IntervalVector> eval(ibex::IntervalVector position);

private:
    std::vector<ibex::Function*> m_functions;
    omp_lock_t  m_lock_dynamics; // Lock
};
}
#endif // DYNAMICS_FUNCTION_H
