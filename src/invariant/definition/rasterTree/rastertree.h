#ifndef RASTERTREE_H
#define RASTERTREE_H

#include <array>
#include <vector>

namespace invariant {

template<typename _Tp=short int, size_t _n=1>
class RasterTree
{
public:
    /**
     * @brief RasterTree constructor
     * @param position
     * @param limit_bisection
     * @param previmer
     * @param leaf_list
     */
    RasterTree(const std::vector<std::array<int, 2>> &position, std::vector<RasterTree *> &leaf_list, std::vector<std::vector<std::array<int, 2>>> & leaf_position);

    /**
     * @brief NodeCurrent destructor
     */
    ~RasterTree();

    /**
     * @brief Return if this NodeCurrent is a leaf
     * @return
     */
    bool is_leaf() const;

    /**
     * @brief Fill the tree
     * @param position
     * @param val_min
     * @param val_max
     */
    bool fill_tree(const std::vector<std::array<int, 2>> &position, _Tp *val_min, _Tp *val_max);

    /**
     * @brief Eval the vector filed at this position
     * @param position
     * @return
     */
    void eval(const std::vector<std::array<int, 2>> &target, const std::vector<std::array<int, 2>> &position, _Tp *val_min, _Tp *val_max) const;

    /**
     * @brief Set the node val
     * @param val_min
     * @param val_max
     */
    void set_node_val(_Tp *val_min, _Tp *val_max, bool valid_data=true);

private:

    /**
     * @brief Compute the union of min/max
     * @param val_min
     * @param val_max
     */
    void union_vector(_Tp *val_min, _Tp *val_max) const;

    /**
     * @brief Bisector
     * @param position
     * @param p1
     * @param p2
     */
    size_t bisector(const std::vector<std::array<int, 2>> &position, std::vector<std::array<int, 2>> &p1, std::vector<std::array<int, 2>> &p2) const;

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

    _Tp m_val_min[_n]; // signed short
    _Tp m_val_max[_n];
    signed char m_bisection_axis = -1; // -1 if leaf
    bool m_valid_data = true;

    RasterTree<_Tp, _n> * m_children_first;
    RasterTree<_Tp, _n> * m_children_second;

};

template<typename _Tp, size_t _n>
bool RasterTree<_Tp, _n>::is_leaf() const{
    return (m_bisection_axis == -1)?true:false;
}

template<typename _Tp, size_t _n>
void RasterTree<_Tp, _n>::set_node_val(_Tp *val_min, _Tp *val_max, bool valid_data){
    std::copy_n(val_min, _n, m_val_min);
    std::copy_n(val_max, _n, m_val_max);
    m_valid_data = valid_data;
}

}
#include "rastertree.tpp"
#endif // RASTERTREE_H
