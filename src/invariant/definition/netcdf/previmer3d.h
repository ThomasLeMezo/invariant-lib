#ifndef PREVIMER3D_H
#define PREVIMER3D_H

#include <string>
#include "nodecurrent3d.h"
#include "dynamics.h"
#include <utility>

namespace invariant {

class NodeCurrent3D; // Friendchip
class PreviMer3D : public invariant::Dynamics
{
public:
    /**
     * @brief PreviMer constructor
     * @param file_name
     */
    PreviMer3D(const std::string &file_directory, const ibex::IntervalVector &search_space, std::vector<double> grid_size, const std::vector<double> &limit_bisection, int stop_level);

    /**
     * @brief PreviMer destructor
     */
    ~PreviMer3D();

    /**
     * @brief Eval a vector field
     * @param position
     * @return
     */
    const std::vector<ibex::IntervalVector> eval(const ibex::IntervalVector &position);

    /**
     * @brief Get the search space associated with the current vector field
     * @return
     */
    const ibex::IntervalVector& get_search_space();

    const std::vector<std::vector<std::vector<short>> >& get_raw_u();

    const std::vector<std::vector<std::vector<short> > > &get_raw_v();

    short get_fill_value();

    const std::vector<size_t>& get_size();

    std::vector<double> get_grid_size();

    std::pair<ibex::IntervalVector, ibex::IntervalVector> bisect_largest_first(const ibex::IntervalVector &position);

private:
    /**
     * @brief Get all the file names in a directory
     * @param dir
     * @param files
     */
    int get_file_list(std::string dir, std::vector<std::string> &files);

private:

    /**
     * @brief Get the value of the vector field at a point from raw data
     * @param position
     * @return
     */
    const ibex::IntervalVector& get_vector_field_at_point(std::vector<double> position);

    /**
     * @brief Fill the leafs of the node tree
     */
    void fill_leafs(const std::vector<std::vector<std::vector<short>>> &raw_u_t, const std::vector<std::vector<std::vector<short>>> &raw_v_t);

private:
    float m_scale_factor = 0;
    short m_fill_value = 0;
    int m_dim = 0;
    std::vector<size_t> m_size;

    NodeCurrent3D *m_node_current;
    std::vector<NodeCurrent3D *> m_leaf_list;

    std::vector<double> m_grid_size;

    std::vector<double> m_limit_bisection;
    std::vector<double> m_ratio_dimension;
};

inline short PreviMer3D::get_fill_value(){
    return m_fill_value;
}

inline const std::vector<size_t>& PreviMer3D::get_size(){
    return m_size;
}

inline std::vector<double> PreviMer3D::get_grid_size(){
    return m_grid_size;
}

}

#endif // PREVIMER3D_H
