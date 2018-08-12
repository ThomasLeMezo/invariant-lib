#ifndef DATASETNODE_H
#define DATASETNODE_H

#include <array>
#include <vector>
#include <fstream>
#include "datasetvirtualnode.h"

namespace invariant {

template<typename _Tp=short int, size_t _n=1>
class DataSetNode : public DataSetVirtualNode
{
public:
    /**
     * @brief DataSetNode constructor
     * @param position
     * @param leaf_list
     */
    DataSetNode(const std::vector<std::array<int, 2> > &position, std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > > &leaf_list);

    /**
     * @brief DataSetNode constructor for deserialization
     * @param binFile
     */
    DataSetNode(std::ifstream& binFile);
    /**
     * @brief DataSetNode destructor
     */
    ~DataSetNode();

    /**
     * @brief Return true if the node is a leaf
     * @return
     */
    bool is_leaf() const;

    /**
     * @brief Return true if the data is valid
     * @return
     */
    bool is_valid_data() const;

    /**
     * @brief Fill the tree
     */
    bool fill_tree();

    /**
     * @brief Eval the vector filed at this position
     * @param target
     * @param position
     * @param data
     */
    void eval(const std::vector<std::array<int, 2>> &target, const std::vector<std::array<int, 2>> &position, std::array<std::array<_Tp, 2>, _n>& data) const;

    /**
     * @brief invert_position
     * @param target
     * @param position
     * @param data
     */
    void eval_invert(std::vector<std::array<int, 2>> &target, const std::vector<std::array<int, 2>> &position, const std::array<std::array<_Tp, 2>, _n> &data) const;

    /**
     * @brief Set the node val
     * @param data
     * @param valid_data
     */
    void set_node_val(const std::array<std::array<_Tp, 2>, _n> &data, bool valid_data=true);

    /**
     * @brief get_data
     * @return
     */
    const std::array<std::array<_Tp, 2>, _n> &get_data() const;

    /**
     * @brief Serialize Raster
     * @param binFile
     */
    void serialize(std::ofstream& binFile) const;

    /**
     * @brief get_number_node
     * @return
     */
    size_t get_number_node() const;

    /**
     * @brief get_number_leaf
     * @return
     */
    size_t get_number_leaf() const;

private:

    /**
     * @brief is_empty_position
     * @param p1
     * @return
     */
    bool is_empty_position(const std::vector<std::array<int, 2>> &p1) const;

    /**
     * @brief get_number_node_private
     * @return
     */
    void get_number_node_private(size_t &nb) const;

    /**
     * @brief get_number_leaf_private
     * @param nb
     */
    void get_number_leaf_private(size_t &nb) const;

    /**
     * @brief Compute the union of min/max and modify this
     */
    void union_this(const std::array<std::array<_Tp, 2>, _n>& data);

    /**
     * @brief Compute the union of min/max and modify data
     */
    void union_data(std::array<std::array<_Tp, 2>, _n>& data) const;

    /**
     * @brief union_position
     * @param p1
     * @param p2
     */
    void union_position(std::vector<std::array<int, 2>>& p1, const std::vector<std::array<int, 2>>& p2) const;

    /**
     * @brief is_inter_empty_data
     * @param d1
     * @param d2
     * @return
     */
    bool is_inter_empty_data(const std::array<std::array<_Tp, 2>, _n> &d1, const std::array<std::array<_Tp, 2>, _n> &d2) const;

    /**
     * @brief is_subset_data
     * @param d1
     * @param d2
     * @return
     */
    bool is_subset_data(const std::array<std::array<_Tp, 2>, _n> &d1, const std::array<std::array<_Tp, 2>, _n> &d2) const;

    /**
     * @brief Bisector
     * @param position
     * @param p1
     * @param p2
     */
    size_t bisector(const std::vector<std::array<int, 2>> &position, std::vector<std::array<int, 2>> &p1, std::vector<std::array<int, 2>> &p2) const;
    size_t bisector(const std::vector<std::array<int, 2>> &position, std::vector<std::array<int, 2>> &p1, std::vector<std::array<int, 2>> &p2, const size_t &l_dim) const;

    /**
     * @brief is_inter_empty
     * @param p1
     * @param p2
     * @return
     */
    bool is_inter_empty(const std::vector<std::array<int, 2>> &p1, const std::vector<std::array<int, 2>> &p2) const;

    /**
     * @brief is_subset
     * @param p1
     * @param p2
     * @return
     */
    bool is_subset(const std::vector<std::array<int, 2>> &p1, const std::vector<std::array<int, 2>> &p2) const;

private:

    std::array<std::array<_Tp, 2>, _n> m_data;
    signed char m_bisection_axis = -1; // -1 if leaf
    bool m_valid_data = true;

    DataSetNode<_Tp, _n> * m_children_first = nullptr;
    DataSetNode<_Tp, _n> * m_children_second = nullptr;

};

template<typename _Tp, size_t _n>
size_t DataSetNode<_Tp, _n>::get_number_node() const{
    size_t nb = 0;
    get_number_node_private(nb);
    return nb;
}

template<typename _Tp, size_t _n>
size_t DataSetNode<_Tp, _n>::get_number_leaf() const{
    size_t nb = 0;
    get_number_leaf_private(nb);
    return nb;
}

template<typename _Tp, size_t _n>
bool DataSetNode<_Tp, _n>::is_leaf() const{
    return (m_bisection_axis == -1)?true:false;
}

template<typename _Tp, size_t _n>
bool DataSetNode<_Tp, _n>::is_valid_data() const{
    return m_valid_data;
}

template<typename _Tp, size_t _n>
void DataSetNode<_Tp, _n>::set_node_val(const std::array<std::array<_Tp, 2>, _n> &data, bool valid_data){
    m_data = data;
    m_valid_data = valid_data;
}

template<typename _Tp, size_t _n>
const std::array<std::array<_Tp, 2>, _n>& DataSetNode<_Tp, _n>::get_data() const{
    return m_data;
}
}

#include "datasetnode.tpp"
#endif // DATASETNODE_H
