#ifndef PREVIMER3D_H
#define PREVIMER3D_H

#include <string>
#include <ibex/ibex_IntervalVector.h>
#include "rasterTree/rastertree.h"
#include "dynamics.h"
#include <utility>
#include <array>

namespace invariant {

class NodeCurrent3D; // Friendchip
class PreviMer3D : public invariant::Dynamics
{
public:
    /**
     * @brief PreviMer constructor
     * @param file_name
     */
    PreviMer3D(const std::string &file_xml, const std::array<std::array<size_t, 2>, 2> &grid_limits);

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
    const ibex::IntervalVector& get_search_space() const;

    /**
     * @brief get_raw_u
     * @return
     */
    const std::vector<std::vector<std::vector<short>> >& get_raw_u();

    /**
     * @brief get_raw_v
     * @return
     */
    const std::vector<std::vector<std::vector<short> > > &get_raw_v();

    /**
     * @brief get_fill_value
     * @return
     */
    short get_fill_value();

    /**
     * @brief get_grid_conversion
     * @param dim
     * @return
     */
    const double &get_grid_conversion(size_t dim) const;

    /**
     * @brief get_grid_conversion
     * @return
     */
    std::vector<double> get_grid_conversion();

private:

    /**
     * @brief load_data
     * @param file_directory
     * @param raw_u_t
     * @param raw_v_t
     */
    void load_data(const std::string &file_xml, std::vector<std::vector<std::vector<short int>>> &raw_u_t,
                   std::vector<std::vector<std::vector<short int>>> &raw_v_t,
                   const std::array<std::array<size_t, 2>, 2> &grid_limits);

    /**
     * @brief Get the value of the vector field at a point from raw data
     * @param position
     * @return
     */
    const ibex::IntervalVector& get_vector_field_at_point(std::vector<double> position);

    /**
     * @brief Conversion function (real position to raster)
     * @param position
     * @return
     */
    const std::vector<std::array<int, 2>> & conversion_function(ibex::IntervalVector position);

    /**
     * @brief Fill the leafs of the node tree
     */
    void fill_leafs(const std::vector<std::vector<std::vector<short> > > &raw_u_t, const std::vector<std::vector<std::vector<short> > > &raw_v_t);

private:
    float m_scale_factor = 0;
    short m_fill_value = 0;
    short m_min_valid, m_max_valid;
    ibex::IntervalVector m_search_space;

    const vector<double> m_grid_conversion = {15.0*60.0, 250.0, 250.0};

    size_t m_offset_i = 0;
    size_t m_offset_j = 0;

    RasterTree<signed short, 2> *m_node_current;
    vector<pair<RasterTree<signed short int, 2>*, vector<array<int, 2>>>> m_leaf_list;
    std::vector<std::array<int, 2>> m_node_root_position;
};

inline short PreviMer3D::get_fill_value(){
    return m_fill_value;
}

inline std::vector<double> PreviMer3D::get_grid_conversion(){
    return m_grid_conversion;
}

inline const double& PreviMer3D::get_grid_conversion(size_t dim) const{
    return m_grid_conversion[dim];
}

inline const ibex::IntervalVector& PreviMer3D::get_search_space()const{
    return m_search_space;
}

}

#endif // PREVIMER3D_H
