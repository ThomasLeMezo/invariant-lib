#ifndef DYNAMICS_H
#define DYNAMICS_H

#include <ibex/ibex_IntervalVector.h>

namespace invariant {

/**
 * @brief The DYNAMICS_SENS enum
 * Dynamics::FWD : propagation or contraction in the sens of the vector field
 * Dynamics::BWD : propagation or contraction in the opposite sens of the vector field
 * Dynamics::FWD_BWD : propagation or contraction in both sens
 */

class Dynamics
{
public:

    enum DYNAMICS_SENS{FWD, BWD, FWD_BWD};

    /**
     * @brief Dynamics
     */
    Dynamics(DYNAMICS_SENS sens);

    virtual const std::vector<ibex::IntervalVector> eval(const ibex::IntervalVector& position) =0;

    DYNAMICS_SENS get_sens() const;

private:
    DYNAMICS_SENS m_dynamics_sens;
};

inline Dynamics::DYNAMICS_SENS Dynamics::get_sens() const{
    return m_dynamics_sens;
}

}

#endif // DYNAMICS_H
