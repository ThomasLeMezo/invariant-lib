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
    Face(const ibex::IntervalVector &coordinates, const ibex::IntervalVector &orientation, Pave* p);
    Face(Pave* p);

    ~Face();

    /**
     * @brief Return the coordinates of the Face
     * @return an interval vector of the coordinates
     */
    ibex::IntervalVector coordinates() const;

    void serialize(std::ofstream &binFile) const;
    void deserialize(std::ifstream& binFile);

    bool operator==(const Face& f) const;
    bool operator!=(const Face& f) const;

    ibex::IntervalVector orientation() const;

private:
    /** Class Variable **/
    ibex::IntervalVector      m_coordinates; // Face coordinates

    /**
     * @brief Orientation of the Face :
     * [1], [0] or [0,1] for each dimension according to the position of
     * the Face in the Pave
     */
    ibex::IntervalVector      m_orientation;
    Pave*                     m_pave;
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
