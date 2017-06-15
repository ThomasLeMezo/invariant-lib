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
    mutable Pave*                     m_pave = NULL;
    std::vector<Face *>               m_neighbors;
};

/**
     * @brief Overload of the operator <<
     * @param stream
     * @param Face
     * @return
     */
std::ostream& operator<< (std::ostream& stream, const Face& f);

/**
     * @brief Overloading of the operator << for a vector of Faces
     * @param stream
     * @param l
     * @return
     */
std::ostream& operator<< (std::ostream& stream, const std::vector<Face*> &l);
}

/// ******** inline functions ******* //

namespace invariant{
inline const ibex::IntervalVector& Face::get_position() const{
    return m_position;
}

inline const ibex::IntervalVector& Face::get_orientation() const{
    return m_orientation;
}

inline const std::vector<Face *> &Face::get_neighbors() const{
    return m_neighbors;
}

inline std::ostream& operator<< (std::ostream& stream, const Face& f) {
    stream << f.get_position();
    return stream;
}

inline const bool Face::is_equal(const Face& f) const{
    if(m_position == f.get_position() && this->m_orientation==f.get_orientation())
        return true;
    else
        return false;
}

}

#endif // FACE_H
