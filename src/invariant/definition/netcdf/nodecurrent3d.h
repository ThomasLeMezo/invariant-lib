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
    NodeCurrent3D(const ibex::IntervalVector &position, const std::vector<double> &limit_bisection, PreviMer3D *previmer, std::vector<NodeCurrent3D *> &leaf_list, std::vector<ibex::IntervalVector> & leaf_position);

    /**
     * @brief NodeCurrent destructor
     */
    ~NodeCurrent3D();

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
    void compute_vector_field_tree(const ibex::IntervalVector &position, short &min_u, short &max_u, short &min_v, short &max_v);

    /**
     * @brief Eval the vector filed at this position
     * @param position
     * @return
     */
    void eval(const ibex::IntervalVector& target, const ibex::IntervalVector& position, short &min_u, short &max_u, short &min_v, short &max_v) const;

    /**
     * @brief Set the vector field of this node
     * @param min_u
     * @param max_u
     * @param min_v
     * @param max_v
     */
    void set_vector_field(short min_u, short max_u, short min_v, short max_v);

private:

    /**
     * @brief Compute the union of min/max vector field
     * @param min_u
     * @param max_u
     * @param min_v
     * @param max_v
     */
    void union_vector(short &min_u, short &max_u,short &min_v, short &max_v) const;

private:
//    ibex::IntervalVector m_position;
//    ibex::IntervalVector m_vector_field;

    short m_min_u = 32767;
    short m_min_v = 32767;
    short m_max_u = -32767;
    short m_max_v = -32767;

    signed char m_bisection_axis = -1; // -1 if leaf

    std::pair<NodeCurrent3D *, NodeCurrent3D *> m_children;

    PreviMer3D *m_previmer;

};

inline bool NodeCurrent3D::is_leaf() const{
    return (m_bisection_axis == -1)?true:false;
}
inline std::pair<NodeCurrent3D *, NodeCurrent3D *> NodeCurrent3D::get_children() const{
    return m_children;
}

inline void NodeCurrent3D::set_vector_field(short min_u, short max_u, short min_v, short max_v){
    m_min_u = min_u;
    m_min_v = min_v;
    m_max_u = max_u;
    m_max_v = max_v;
}

}

#endif // NODECURRENT3D_H
