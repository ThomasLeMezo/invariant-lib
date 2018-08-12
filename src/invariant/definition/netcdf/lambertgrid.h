#ifndef LAMBERTGRID_H
#define LAMBERTGRID_H

#include <string>
#include <vector>
#include "dataSet/datasetnode.h"
#include "dataSet/datasetvirtualnode.h"

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
     * @brief get_X
     * @return
     */
    const std::vector<std::vector<double>>& get_X() const;

    /**
     * @brief get_Y
     * @return
     */
    const std::vector<std::vector<double>>& get_Y() const;


private:
    std::vector<std::vector<double>> m_X;
    std::vector<std::vector<double>> m_Y;

    DataSetVirtualNode *m_dataSet_U = nullptr;
    DataSetVirtualNode *m_dataSet_V = nullptr;
};

inline const std::vector<std::vector<double>>& LambertGrid::get_X() const{
    return m_X;
}

inline const std::vector<std::vector<double>>& LambertGrid::get_Y() const{
    return m_Y;
}

}

#endif // LAMBERTGRID_H
