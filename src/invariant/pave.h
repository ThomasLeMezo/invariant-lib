#ifndef PAVE_H
#define PAVE_H

#include <ibex.h>
#include "face.h"
#include <fstream>
#include "graph.h"

namespace invariant {
class Face; // declared only for friendship
class Graph; // declared only for friendship

class Pave
{
public:
    /**
     * @brief Create a Pave with coordinates
     * @param coordinates of the Pave
     */
    Pave(const ibex::IntervalVector &coordinates, Graph* g);
    Pave(Graph* g);
    Pave():m_coordinates(0), m_faces(0){}

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

    size_t getSerialization_id() const;
    void setSerialization_id(size_t &value);

    void serialize(std::ofstream &binFile) const;
    void deserialize(std::ifstream& binFile);

    bool operator==(const Pave& p) const;
    bool operator!=(const Pave& p) const;
    const std::array<Face*, 2>& operator[](std::size_t i) const;

private:

    /** Class Variable **/
    ibex::IntervalVector      m_coordinates; // Pave coordinates
    std::vector< std::array<Face*, 2>> m_faces; // Faces of the Pave
    Graph*                    m_graph;

    size_t m_serialization_id;
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
