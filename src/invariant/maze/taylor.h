#ifndef TAYLOR_H
#define TAYLOR_H

#include "ibex/ibex_IntervalVector.h"
#include "ibex/ibex_Interval.h"

namespace invariant {

/**
 * @brief compute_taylor
 * @param t
 * @param a
 * @param b
 * @param c
 * @return
 */
ibex::IntervalVector taylor_compute_point(const ibex::Interval &t, const ibex::IntervalVector &a, const ibex::IntervalVector &b, const ibex::IntervalVector &c);

/**
 * @brief contract_t
 * @param t
 * @param a
 * @param b
 * @param c
 * @return
 */
ibex::Interval taylor_contract_t(const ibex::Interval &t, const ibex::Interval& a, const ibex::Interval& b, const ibex::Interval& c);

/**
 * @brief contract_trajectory
 * @param box
 * @param t
 * @param a
 * @param b
 * @param c
 * @param sens
 * @return
 */
ibex::IntervalVector taylor_contract_trajectory(const ibex::IntervalVector &box, const ibex::Interval &t, const ibex::IntervalVector &a, const ibex::IntervalVector &b, const ibex::IntervalVector &c, const size_t &sens);

/**
 * @brief contrat_box
 * @param pave
 * @param box
 * @param a
 * @param b
 * @param c
 * @return
 */
ibex::IntervalVector taylor_contrat_box(const ibex::IntervalVector &box, const ibex::IntervalVector &a, const ibex::IntervalVector &b, const ibex::IntervalVector &c);

}

#endif // TAYLOR_H
