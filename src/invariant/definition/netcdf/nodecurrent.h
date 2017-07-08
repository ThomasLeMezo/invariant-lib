#ifndef NODECURRENT_H
#define NODECURRENT_H

#include <ibex.h>

class NodeCurrent
{
public:
    /**
     * @brief NodeCurrent constructor
     * @param position
     */
    NodeCurrent(const ibex::IntervalVector &position, double epsilon_bisection);

    /**
     * @brief Get the position of this NodeCurrent
     * @return
     */
    const ibex::IntervalVector& get_position() const;

    /**
     * @brief Get the vector field associated with this NodeCurrent
     * @return
     */
    const ibex::IntervalVector& get_vector_field() const;

    /**
     * @brief Set the vector field associated with this NodeCurrent
     * @param iv
     */
    void set_vector_field(const ibex::IntervalVector& iv);

    /**
     * @brief Return if this NodeCurrent is a leaf
     * @return
     */
    bool is_leaf() const;

    /**
     * @brief Get the children of this NodeCurrent
     * @return
     */
    std::pair<NodeCurrent *, NodeCurrent *> get_children() const;

    /**
     * @brief Compute the vector field if this NodeCurrent is not a leaf
     * (union of its children vector field)
     * @return
     */
    const ibex::IntervalVector &compute_vector_field_tree();

    /**
     * @brief Eval the vector filed at this position
     * @param position
     * @return
     */
    const ibex::IntervalVector eval(const ibex::IntervalVector& position);

    /**
     * @brief Fill leafs with current
     */
    void fill_leafs(short *raw_u, short *raw_v, const size_t& i_max, const float &scale_factor, const short& fill_value);

    /**
     * @brief Get the leaf list of this NodeCurrent (list of all leaf children nodes)
     * @return
     */
    const std::vector<NodeCurrent *> &get_leaf_list() const;

private:
    ibex::IntervalVector m_position;
    ibex::IntervalVector m_vector_field;

    std::pair<NodeCurrent *, NodeCurrent *> m_children;
    bool                m_leaf = false;

    std::vector<NodeCurrent *> m_leaf_list;

};

inline const ibex::IntervalVector& NodeCurrent::get_position() const{
    return m_position;
}

inline const ibex::IntervalVector& NodeCurrent::get_vector_field() const{
    return m_vector_field;
}

inline void NodeCurrent::set_vector_field(const ibex::IntervalVector& iv){
    m_vector_field = iv;
}

inline bool NodeCurrent::is_leaf() const{
    return m_leaf;
}
inline std::pair<NodeCurrent *, NodeCurrent *> NodeCurrent::get_children() const{
    return m_children;
}

inline const std::vector<NodeCurrent *>& NodeCurrent::get_leaf_list() const{
    return m_leaf_list;
}

#endif // NODECURRENT_H
