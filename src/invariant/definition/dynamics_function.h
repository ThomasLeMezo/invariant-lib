#ifndef DYNAMICS_FUNCTION_H
#define DYNAMICS_FUNCTION_H

#include "dynamics.h"
#include <ibex.h>

namespace invariant {
class Dynamics_Function: public Dynamics
{
public:
    Dynamics_Function(const std::vector<ibex::Function *> functions);
    Dynamics_Function(ibex::Function *functions);

    const std::vector<ibex::IntervalVector> eval(ibex::IntervalVector position);

private:
    std::vector<ibex::Function*> m_functions;
};
}
#endif // DYNAMICS_FUNCTION_H
