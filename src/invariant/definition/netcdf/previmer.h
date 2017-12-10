#ifndef PREVIMER_H
#define PREVIMER_H

#include <ibex_IntervalVector.h>
#include <string>
#include "nodecurrent.h"
#include "dynamics.h"

namespace invariant {

class PreviMer : public invariant::Dynamics
{
public:
    /**
     * @brief PreviMer constructor
     * @param file_name
     */
    PreviMer(const std::string &file_name);

    /**
     * @brief Eval a vector field
     * @param position
     * @return
     */
    const std::vector<ibex::IntervalVector> eval(const ibex::IntervalVector &position);

    const std::vector<ibex::IntervalVector> eval_d1(const ibex::IntervalVector &position){
        std::vector<ibex::IntervalVector> empty;
        return empty;
    }

    /**
     * @brief Get the search space associated with the current vector field
     * @return
     */
    const ibex::IntervalVector& get_search_space();

    const std::vector<std::vector<short> >& get_raw_u();

    const std::vector<std::vector<short> >& get_raw_v();

    size_t get_j_max();

    size_t get_i_max();

    short get_fill_value();

private:

    /**
     * @brief Get the value of the vector field at a point from raw data
     * @param position
     * @return
     */
    const ibex::IntervalVector& get_vector_field_at_point(std::vector<double> position);

private:
    std::vector<std::vector<short>> m_raw_u;
    std::vector<std::vector<short>> m_raw_v;
    float m_scale_factor = 0;
    short m_fill_value = 0;
    int m_dim = 0;
    size_t m_i_max = 0;
    size_t m_j_max = 0;

    NodeCurrent *m_node_current;

};

inline const std::vector<std::vector<short>>& PreviMer::get_raw_u(){
    return m_raw_u;
}

inline const std::vector<std::vector<short>>& PreviMer::get_raw_v(){
    return m_raw_v;
}

inline size_t PreviMer::get_j_max(){
    return m_j_max;
}

inline size_t PreviMer::get_i_max(){
    return m_i_max;
}

inline short PreviMer::get_fill_value(){
    return m_fill_value;
}

}

#endif // PREVIMER_H
