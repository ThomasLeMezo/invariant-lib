#ifndef FACE_H
#define FACE_H

#include <ibex.h>
#include <fstream>
#include "pave.h"

namespace invariant {
class Pave; // declared only for friendship
class Face
{
public:

    /**
     * @brief Face constructor with position, orientation and a Pave
     * @param position of the Pave
     * @param orientation Vector ([0], [1] or [0,1] for each dimension)
     * @param p
     */
    Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, Pave* p);

    /**
     * @brief Face empty constructor with a Pave
     * (used for serialization)
     * @param p
     */
    Face(Pave* p);

    /**
     * @brief Face destructor
     */
    ~Face();

    /**
     * @brief Return the position of the Face
     * @return an interval vector of the position
     */
    const ibex::IntervalVector &get_position() const;

    /**
     * @brief Face serialization
     * @param binFile
     */
    void serialize(std::ofstream &binFile) const;

    /**
     * @brief Face deserialization
     * @param binFile
     */
    void deserialize(std::ifstream& binFile);

    /**
     * @brief Test equality between two Faces
     * @param f
     * @return
     */
    const bool is_equal(const Face& f) const;

    /**
     * @brief Return orientation of the Face
     * @return
     */
    const ibex::IntervalVector &get_orientation() const;

    /**
     * @brief Add new neighbor to the Face if the position intersection is not empty
     * @param f
     */
    void add_neighbor(Face *f);

    /**
     * @brief Remove a neighbor from neighbors list
     * @param f
     */
    void remove_neighbor(const Face *f);

    /**
     * @brief Get neighbors list
     * @return
     */
    const std::vector<Face *>& get_neighbors() const;

private:
    /** Class Variable **/
    mutable ibex::IntervalVector      m_position; // Face position
    mutable ibex::IntervalVector      m_orientation; // Orientation of the Face : [1], [0] or [0,1]
                                                     // for each dimension according to the position of the Face in the Pave
    mutable Pave*                     m_pave;
    std::vector<Face *>               m_neighbors;
};

    /**
     * @brief Overload of the operator <<
     * @param stream
     * @param Face
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const Face& f);
}

#endif // FACE_H
