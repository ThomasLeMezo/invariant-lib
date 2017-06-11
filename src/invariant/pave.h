#ifndef PAVE_H
#define PAVE_H

#include <ibex.h>
#include <face.h>

namespace invariant {
class Pave
{
public:
    /**
     * @brief Create a Pave with coordinates
     * @param coordinates of the Pave
     */
    Pave(const ibex::IntervalVector &coordinates);

    ~Pave();

    /**
     * @brief Return the coordinates of the Pave
     * @return an interval vector of the coordinates
     */
    ibex::IntervalVector coordinates() const;

    /**
     * @brief Return the array of an array of Faces of the Pave
     * @return A two arrays of pointer to the faces
     */
    std::vector< std::array<Face *, 2>> faces() const;

private:

    /** Class Variable **/
    ibex::IntervalVector      m_coordinates; // Pave coordinates
    std::vector< std::array<Face*, 2>> m_faces; // Faces of the Pave
};

    /**
     * @brief Overloading of operator <<
     * @param stream
     * @param Pave
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const Pave& p);
}

#endif // PAVE_H
