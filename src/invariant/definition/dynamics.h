#ifndef DYNAMICS_H
#define DYNAMICS_H

#include <ibex_IntervalVector.h>
#include <ibex_IntervalMatrix.h>
#include <ibex_Sep.h>
#include <map>

namespace invariant {

/**
 * @brief The DYNAMICS_SENS enum
 * FWD : propagation or contraction in the sens of the vector field
 * BWD : propagation or contraction in the opposite sens of the vector field
 * FWD_BWD : propagation or contraction in both sens
 */

enum DYNAMICS_SENS{FWD, BWD, FWD_BWD};

class Dynamics
{
public:
    /**
     * @brief Dynamics
     */
    Dynamics(DYNAMICS_SENS sens);

    virtual const std::vector<ibex::IntervalVector> eval(const ibex::IntervalVector& position) =0;

    virtual const std::vector<ibex::IntervalMatrix> eval_d1(const ibex::IntervalVector &position) =0;

    virtual std::vector<ibex::Sep*> get_hybrid_guard(){
        return std::vector<ibex::Sep*>();
    }

    virtual std::map<ibex::Sep*, std::array<ibex::Function*, 2>> get_hybrid_reset(){
        std::cout << "ERROR" << std::endl;
        return std::map<ibex::Sep*, std::array<ibex::Function*, 2>>();
    }

    DYNAMICS_SENS get_sens() const;

    void set_sens(DYNAMICS_SENS sens);

protected:
    DYNAMICS_SENS m_dynamics_sens;
};

inline DYNAMICS_SENS Dynamics::get_sens() const{
    return m_dynamics_sens;
}

inline void Dynamics::set_sens(DYNAMICS_SENS sens){
    m_dynamics_sens = sens;
}

}

#endif // DYNAMICS_H
