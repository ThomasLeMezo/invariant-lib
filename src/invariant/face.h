#ifndef FACE_H
#define FACE_H

#include <ibex.h>

namespace invariant {
class Face
{
public:
    Face(const ibex::IntervalVector &coordinates, const ibex::IntervalVector &orientation);

    /**
     * @brief Return the coordinates of the Face
     * @return an interval vector of the coordinates
     */
    ibex::IntervalVector coordinates() const;



private:

    /** Class Variable **/
    ibex::IntervalVector      m_coordinates; // Face coordinates

    /**
     * @brief Orientation of the Face :
     * [1], [0] or [0,1] for each dimension according to the position of
     * the Face in the Pave
     */
    ibex::IntervalVector      m_orientation;
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
