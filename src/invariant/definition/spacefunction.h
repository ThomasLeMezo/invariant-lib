#ifndef SPACEFUNCTION_H
#define SPACEFUNCTION_H

#include <omp.h>
#include <vector>

#include <ibex/ibex_Function.h>
#include <ibex_IntervalVector.h>
#include <ibex_IntervalMatrix.h>
#include <ibex_Interval.h>
#include <ibex/ibex_Sep.h>

namespace invariant {

class SpaceFunction : public ibex::Function
{
public:
    SpaceFunction();

    ~SpaceFunction(){}

    /**
     * @brief Eval a vector field
     * @param position
     * @return
     */
    virtual ibex::IntervalVector eval_vector(const ibex::IntervalVector &position) const;

    virtual ibex::Interval eval(const ibex::IntervalVector &position) const {return ibex::Interval::EMPTY_SET;}
    virtual ibex::IntervalMatrix eval_matrix(const ibex::IntervalVector &position) const {return ibex::IntervalMatrix::empty(position.size(), position.size());}

    /**
     * @brief push_back
     * @param f
     * @param sep
     */
    void push_back(ibex::Function *f, ibex::Sep *sep);

private:

    std::vector<ibex::Function *> m_functions_list;
    std::vector<ibex::Sep *> m_separator_list;
};

}

#endif // SPACEFUNCTION_H
