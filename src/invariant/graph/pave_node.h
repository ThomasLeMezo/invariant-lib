#ifndef PAVE_NODE_H
#define PAVE_NODE_H

#include <utility>
#include <ibex_IntervalVector.h>
#include <pave.h>

namespace invariant{
class Pave; // declare for friendchip
class Face; // declare for friendchip
class Pave_node
{
public:
    /**
     * @brief Construct a leaf node of the tree which leans to the subpaving
     * @param p
     */
    Pave_node(Pave *p);

    /**
     * @brief Recursive destructor
     */
    ~Pave_node();

    /**
     * @brief Add childs to the binary tree (this node is set to not be a leaf)
     * @param Pave1
     * @param Pave2
     */
    void add_child(Pave *p1, Pave *p2);

    /**
     * @brief Get all Paves that intersect a box (outer condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_pave_outer(std::vector<Pave*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Paves that are not contracted to emptyness by a given contractor
     * @param vector of Pave*
     * @param NumConstraint to fulfill (>0)
     */
    void get_intersection_pave_outer(std::vector<Pave*> &l, ibex::Ctc &nc);

    /**
     * @brief Get all Paves that are inside a box (inner condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_pave_inner(std::vector<Pave*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Paves that are not contracted by a given contractor
     * @param l
     * @param nc
     */
    void get_intersection_pave_inner(std::vector<Pave*> &l, ibex::Ctc &nc);

    /**
     * @brief Get all Faces that intersect a box (outer condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_face_outer(std::vector<Face*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Faces that are not contracted to emptyness by a given contractor
     * @param vector of Pave*
     * @param NumConstraint to fulfill (>0)
     */
    void get_intersection_face_outer(std::vector<Face*> &l, ibex::Ctc &nc);

    /**
     * @brief Get all Faces that are inside a box (inner condition)
     * @param vector of Pave*
     * @param box
     */
    void get_intersection_face_inner(std::vector<Face*> &l, const ibex::IntervalVector &box);

    /**
     * @brief Get all Faces that are not contracted by a given contractor
     * @param l
     * @param nc
     */
    void get_intersection_face_inner(std::vector<Face*> &l, ibex::Ctc &nc);

    /**
     * @brief Get the pave associated to this pave_node
     * @return
     */
    Pave* get_pave() const;


private:
    bool                                m_leaf;
    mutable Pave*                       m_pave = NULL;
    std::pair<Pave_node *, Pave_node *> m_child;
    const ibex::IntervalVector          m_position;
};

}
#endif // PAVE_NODE_H
