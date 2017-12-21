#ifndef DYNAMICSINCLUSIONFUNCTION_H
#define DYNAMICSINCLUSIONFUNCTION_H

#include "dynamics_function.h"
#include <ibex_IntervalVector.h>
#include <ibex_Function.h>
#include "omp.h"

namespace invariant {
class DynamicsInclusionFunction: public Dynamics_Function
{
public:   
    /**
     * @brief DynamicsInclusionFunction
     * @param functions
     * @param inclusion_parameter
     * @param sens
     * @param taylor
     */
    DynamicsInclusionFunction(const std::vector<ibex::Function *> functions, const ibex::IntervalVector &inclusion_parameter, const DYNAMICS_SENS sens=FWD, bool taylor=false);

    /**
     * @brief DynamicsInclusionFunction
     * @param f
     * @param inclusion_parameter
     * @param sens
     * @param taylor
     */
    DynamicsInclusionFunction(ibex::Function *f, const ibex::IntervalVector &inclusion_parameter,  const DYNAMICS_SENS sens=FWD, bool taylor=false);

    /**
     * @brief Destructor
     */
//    ~DynamicsInclusionFunction();

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
//    const std::vector<ibex::IntervalMatrix> eval_d1(const ibex::IntervalVector &position);

    /**
     * @brief set_inclusion_parameter
     * @param parameter
     */
    void set_inclusion_parameter(const ibex::IntervalVector &parameter);
    void set_inclusion_parameter(const ibex::Interval &parameter);

    /**
     * @brief get_inclusion_parameter
     * @return
     */
    ibex::IntervalVector get_inclusion_parameter();

private:
    ibex::IntervalVector m_inclusion_parameter;
};
}

#endif // DYNAMICSINCLUSIONFUNCTION_H
