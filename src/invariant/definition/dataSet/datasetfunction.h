#ifndef DATASETFUNCTION_H
#define DATASETFUNCTION_H

#include "ibex_IntervalVector.h"
#include "ibex_Fnc.h"
#include <string>
#include "datasetnode.h"

namespace invariant {

enum DataSetFunction_TYPE {TYPE_SHORT_INT, TYPE_INT, TYPE_DOUBLE, TYPE_CHAR};

class DataSetFunction : public ibex::Fnc
{
public:
    /**
     * @brief DataSetFunction constructor
     * @param root_position
     */
    DataSetFunction(const ibex::IntervalVector &node_root_position, const std::vector<std::array<int, 2> > &root_position, const size_t cell_dim, const DataSetFunction_TYPE type);

    /**
     * @brief DataSetFunction destructor
     */
    ~DataSetFunction();

    /**
     * @brief Load DataSetFunction from a file
     * @param root_position
     */
    DataSetFunction(string file_name);

    /**
     * @brief Implementation of the eval method
     * @param box
     * @return
     */
    ibex::Interval eval(const ibex::IntervalVector& box) const;

    /**
     * @brief Implementation of the eval_vector method
     * @param box
     * @return
     */
    ibex::IntervalVector eval_vector(const ibex::IntervalVector& box) const;

    /**
     * @brief Implementation of the eval_matrix method
     * @param x
     * @return
     */
    ibex::IntervalMatrix eval_matrix(const ibex::IntervalVector& x) const;

    /**
     * @brief Serialization of the DataSetFunction
     * @param file_name
     */
    void serialize(string file_name) const;

    /**
     * @brief root_position
     * @return
     */
    const std::vector<std::array<int, 2> >& get_root_position() const;

    /**
     * @brief Append Data
     */
    void append_data();

private:
    ibex::IntervalVector* m_root_real_position;
    std::vector<std::array<int, 2> > m_root_position;
    DataSetVirtualNode *m_root;

    DataSetFunction_TYPE m_type;
    size_t  m_cell_dim;

    vector<pair<DataSetVirtualNode*, vector<array<int, 2> > > > m_leaf_list;
};

}

#endif // DATASETFUNCTION_H
