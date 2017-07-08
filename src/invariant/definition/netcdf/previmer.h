#ifndef PREVIMER_H
#define PREVIMER_H

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
    const std::vector<ibex::IntervalVector> eval(ibex::IntervalVector position);

    /**
     * @brief Get the search space associated with the current vector field
     * @return
     */
    const ibex::IntervalVector& get_search_space();

    short* get_raw_u();

    short* get_raw_v();

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
    signed short *m_raw_u;
    signed short *m_raw_v;
    float m_scale_factor = 0;
    short m_fill_value = 0;
    int m_dim = 0;
    size_t m_i_max = 0;
    size_t m_j_max = 0;

    NodeCurrent *m_node_current;

};

inline short* PreviMer::get_raw_u(){
    return m_raw_u;
}

inline short* PreviMer::get_raw_v(){
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
