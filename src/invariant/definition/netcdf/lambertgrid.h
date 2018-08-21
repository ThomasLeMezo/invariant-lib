#ifndef LAMBERTGRID_H
#define LAMBERTGRID_H

#include <string>
#include <vector>
#include "dataSet/datasetnode.h"
#include "dataSet/datasetvirtualnode.h"
#include <netcdf>

namespace invariant {

class LambertGrid
{
public:
    /**
     * @brief LambertGrid
     * @param file_name
     */
    LambertGrid(const std::string& file_name);

    /**
     * @brief destructor LambertGrid
     */
    ~LambertGrid();

    /**
     * @brief get_U_X
     * @return
     */
    const std::vector<std::vector<double>>& get_U_X() const;

    /**
     * @brief get_U_Y
     * @return
     */
    const std::vector<std::vector<double>>& get_U_Y() const;

    /**
     * @brief get_V_X
     * @return
     */
    const std::vector<std::vector<double>>& get_V_X() const;

    /**
     * @brief get_V_Y
     * @return
     */
    const std::vector<std::vector<double>>& get_V_Y() const;

    /**
     * @brief eval
     */
    bool eval(const double &x, const double &y, const double &t, double &u, double &v) const;

    /**
     * @brief get_H0_Fill_Value
     * @return
     */
    const short& get_H0_Fill_Value() const;

    const std::vector<std::vector<double>> &get_X() const;
    const std::vector<std::vector<double>> &get_Y() const;
    const std::vector<std::vector<short int>> &get_H0() const;

private:
    /**
     * @brief compute_grid_proj
     * @param dataFile
     */
    void compute_grid_proj(netCDF::NcFile &dataFile);

    /**
     * @brief fill_leaf
     * @param leaf_list
     * @param X
     * @param Y
     */
    void fill_leaf(std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2>>>> &leaf_list,
                   std::vector<std::vector<double>> &X,
                   std::vector<std::vector<double>> &Y);

    /**
     * @brief get_time_grid
     * @param t
     * @return
     */
    size_t get_time_grid(const double &t) const;

private:
    std::vector<std::vector<double>> m_U_X, m_U_Y, m_V_X, m_V_Y, m_X, m_Y;

    DataSetVirtualNode *m_dataSet_U = nullptr;
    DataSetVirtualNode *m_dataSet_V = nullptr;

    std::vector<std::array<int, 2>> m_position_U, m_position_V;

    double m_U_scale_factor, m_U_add_offset;
    double m_V_scale_factor, m_V_add_offset;
    short int m_U_Fill_Value, m_V_Fill_Value;
    short int m_H0_Fill_Value;

    std::vector<std::vector<std::vector<short int>>> m_U, m_V;
    std::vector<std::vector<short int>> m_H0;
    std::vector<double> m_time;
    double m_time_dt;

    double m_distance_max_U_X, m_distance_max_U_Y, m_distance_max_U;
    double m_distance_max_V_X, m_distance_max_V_Y, m_distance_max_V;
};

inline const std::vector<std::vector<double>>& LambertGrid::get_U_X() const{
    return m_U_X;
}

inline const std::vector<std::vector<double>>& LambertGrid::get_U_Y() const{
    return m_U_Y;
}

inline const std::vector<std::vector<double>>& LambertGrid::get_V_X() const{
    return m_V_X;
}

inline const std::vector<std::vector<double>>& LambertGrid::get_V_Y() const{
    return m_V_Y;
}

inline const short int &LambertGrid::get_H0_Fill_Value() const{
    return m_H0_Fill_Value;
}

inline const std::vector<std::vector<double>> &LambertGrid::get_X() const{
    return m_X;
}

inline const std::vector<std::vector<double>> &LambertGrid::get_Y() const{
    return m_Y;
}

inline const std::vector<std::vector<short> >& LambertGrid::get_H0() const{
    return m_H0;
}

}

#endif // LAMBERTGRID_H
