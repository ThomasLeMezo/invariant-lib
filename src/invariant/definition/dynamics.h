#ifndef DYNAMICS_H
#define DYNAMICS_H

#include <ibex.h>

namespace invariant {
class Dynamics
{
public:

    /**
     * @brief Dynamics
     */
    Dynamics();

    virtual const std::vector<ibex::IntervalVector> eval(const ibex::IntervalVector& position) =0;
};
}

#endif // DYNAMICS_H
