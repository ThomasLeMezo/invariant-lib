#ifndef NODECURRENT3D_H
#define NODECURRENT3D_H

#include <ibex.h>
#include "previmer3d.h"

namespace invariant {

class PreviMer3D;
class NodeCurrent3D
{
public:
    /**
     * @brief NodeCurrent3D constructor
     * @param position
     * @param limit_bisection
     * @param previmer
     * @param leaf_list
     */
    NodeCurrent3D(const ibex::IntervalVector &position, const std::vector<double> &limit_bisection, PreviMer3D *previmer, std::vector<NodeCurrent3D *> &leaf_list);

    /**
     * @brief NodeCurrent destructor
     */
    ~NodeCurrent3D();

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
    std::pair<NodeCurrent3D *, NodeCurrent3D *> get_children() const;

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
     * @brief Get the leaf list of this NodeCurrent (list of all leaf children nodes)
     * @return
     */
    const std::vector<NodeCurrent3D *> &get_leaf_list() const;

private:
    ibex::IntervalVector m_position;
    ibex::IntervalVector m_vector_field;

    std::pair<NodeCurrent3D *, NodeCurrent3D *> m_children;
    bool                m_leaf = false;

    PreviMer3D *m_previmer;

};

inline const ibex::IntervalVector& NodeCurrent3D::get_position() const{
    return m_position;
}

inline const ibex::IntervalVector& NodeCurrent3D::get_vector_field() const{
    return m_vector_field;
}

inline void NodeCurrent3D::set_vector_field(const ibex::IntervalVector& iv){
    m_vector_field = iv;
}

inline bool NodeCurrent3D::is_leaf() const{
    return m_leaf;
}
inline std::pair<NodeCurrent3D *, NodeCurrent3D *> NodeCurrent3D::get_children() const{
    return m_children;
}

}

#endif // NODECURRENT3D_H
