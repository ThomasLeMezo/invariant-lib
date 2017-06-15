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
     * @brief Construct a Pave with coordinates and a Graph
     * @param coordinates of the Pave
     */
    Pave(const ibex::IntervalVector &coordinates, Graph* g);

    /**
     * @brief Construct an empty Pave link to a Graph
     * @param g
     */
    Pave(Graph* g);

    /**
     * @brief Construct an empty Pave
     */
    Pave():m_position(0), m_faces(0){}

    /**
     * @brief Pave destructor
     */
    ~Pave();

    /**
     * @brief Return the coordinates of the Pave
     * @return an interval vector of the coordinates
     */
    const ibex::IntervalVector &get_position() const;

    /**
     * @brief Return the array of an array of Faces of the Pave
     * @return A two arrays of pointer to the faces
     */
    const std::vector< std::array<Face *, 2>>& get_faces() const;

    /**
     * @brief Get the Id of the Pave given by the serialization step
     * (used to reconstruct pointers)
     * @return Id
     */
    const size_t& get_serialization_id() const;

    /**
     * @brief Set an Id to the Pave for serialization
     * (used to reconstruct pointers)
     * @param value
     */
    void set_serialization_id(const size_t &value);

    /**
     * @brief Serialize the Pave
     * @param binFile
     */
    void serialize(std::ofstream &binFile) const;

    /**
     * @brief Deserialize the Pave
     * @param binFile
     */
    void deserialize(std::ifstream& binFile);

    /**
     * @brief Test the equality between two paves
     * @param p
     * @return
     */
    const bool is_equal(const Pave& p) const;

    /**
     * @brief Return the two Faces of the Pave in the i-th dimension
     * @param i
     * @return
     */
    const std::array<Face*, 2>& operator[](const std::size_t& i) const;

    /**
     * @brief Bisect the Pave
     * - add the result to the Graph
     * - upate neighbors
     */
    void bisect();

    /**
     * @brief Return true if the Pave have to be bisected
     * (temporary)
     * @return
     */
    const bool request_bisection();

    /**
     * @brief Return the two child Paves after calling bisection
     * @return
     */
    const std::array<Pave *, 2> &getResult_bisected();

    /**
     * @brief Return all the Faces of the Pave in a vector
     * @return
     */
    const std::vector<Face *> &faces_vector();

private:

    /** Class Variable **/
    mutable ibex::IntervalVector      m_position; // Pave coordinates
    mutable std::vector< std::array<Face*, 2>> m_faces; // Faces of the Pave
    mutable std::vector<Face *> m_faces_vector; // Faces of the Pave
    mutable Graph*                    m_graph;

    std::array<Pave*, 2>    m_result_bisected;

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
