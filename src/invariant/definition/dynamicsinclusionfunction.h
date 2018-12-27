#ifndef DYNAMICSINCLUSIONFUNCTION_H
#define DYNAMICSINCLUSIONFUNCTION_H

#include "dynamicsFunction.h"

/**
 * Dynamics Inclusion Function is an extension of Dynamics Function when the function depends of an interval parameters (inclusion function)
 * that can change during the computation
 *
*/

namespace invariant {
class DynamicsInclusionFunction: public DynamicsFunction
{
public:   
    /**
     * @brief DynamicsInclusionFunction
     * @param functions
     * @param inclusion_parameter
     * @param sens
     * @param taylor
     */
    DynamicsInclusionFunction(const std::vector<ibex::Function *> functions, const ibex::IntervalVector &inclusion_parameter, const DYNAMICS_SENS sens=FWD);

    /**
     * @brief DynamicsInclusionFunction
     * @param f
     * @param inclusion_parameter
     * @param sens
     * @param taylor
     */
    DynamicsInclusionFunction(ibex::Function *f, const ibex::IntervalVector &inclusion_parameter,  const DYNAMICS_SENS sens=FWD);

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
