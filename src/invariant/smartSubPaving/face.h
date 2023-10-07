#ifndef FACE_H
#define FACE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <iostream>
#include <fstream>
#include <map>

#include "../serialization/ibex_serialization.h"

#include "pave.h"
#include "door.h"
#include "room.h"
#include "maze.h"

namespace invariant {

using FacePPL = Face<Parma_Polyhedra_Library::C_Polyhedron>;
using FaceIBEX = Face<ibex::IntervalVector>;
using FaceEXP = Face<invariant::ExpBox>;

template <typename _Tp> class Pave;
template <typename _Tp> class Door;
template <typename _Tp> class Room;
template <typename _Tp> class Maze;

template <typename _Tp>
class Face
{
public:

    /**
     * @brief Face constructor with position, orientation and a Pave
     * @param position of the Pave
     * @param orientation Vector ([0], [1] or [0,1] for each dimension)
     * @param p
     */
    Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, Pave<_Tp>* p);

    /**
     * @brief Face empty constructor with a Pave
     * (used for serialization)
     * @param p
     */
    Face<_Tp>(Pave<_Tp>* p);

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
     * @brief Return the position of the Face (templated)
     * @return
     */
    const _Tp get_position_typed() const;

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
    const bool is_equal(const Face<_Tp>& f) const;

    /**
     * @brief Return orientation of the Face
     * @return
     */
    const ibex::IntervalVector &get_orientation() const;

    /**
     * @brief Get the normal vector of the Face
     * @return
     */
    const ibex::IntervalVector &get_normal() const;

    /**
     * @brief Add new neighbor to the Face if the position intersection is not empty
     * @param f
     */
    void add_neighbor(Face<_Tp> *f);

    /**
     * @brief Remove a neighbor from neighbors list
     * @param f
     */
    void remove_neighbor(const Face<_Tp> *f);

    /**
     * @brief Get neighbors list
     * @return
     */
    const std::vector<Face<_Tp> *> &get_neighbors() const;

    /**
     * @brief Getter to the maze/door map
     * @return
     */
    std::map<Maze<_Tp> *, Door<_Tp> *> get_doors();

    /**
     * @brief Add a new door to the map
     * @param maze
     * @param door
     */
    void add_door(Door<_Tp> *door);

    /**
     * @brief Getter to the pave of this face
     * @return
     */
    Pave<_Tp> * get_pave() const;

    /**
     * @brief Return true if this face has no neighbors
     * @return
     */
    bool is_border() const;

    /**
     * @brief Analyze if this Face is a border face
     */
    bool analyze_border();

    /**
     * @brief Set this Face as a border face
     */
    void set_border();
    
private:
    /** Class Variable **/
    mutable ibex::IntervalVector      m_position; // Face position
    mutable ibex::IntervalVector      m_orientation; // Orientation of the Face : [1], [0] or [0,1]
    mutable ibex::IntervalVector      m_normal; // Normal vector of the face
    _Tp                               m_position_typed; // Face position
    // for each dimension according to the position of the Face in the Pave
    mutable Pave<_Tp>*                m_pave = nullptr;
    std::vector<Face<_Tp> *>          m_neighbors;
    bool                              m_border = false;

    std::map<Maze<_Tp>*, Door<_Tp>*>  m_doors;

    omp_lock_t m_write_neighbors;

};

/**
     * @brief Overload of the operator <<
     * @param stream
     * @param Face
     * @return
     */
template <typename _Tp>
std::ostream& operator<< (std::ostream& stream, const Face<_Tp>& f);

/**
     * @brief Overloading of the operator << for a vector of Faces
     * @param stream
     * @param l
     * @return
     */
template <typename _Tp>
std::ostream& operator<< (std::ostream& stream, const std::vector<Face<_Tp>*> &l);
}

/// ******** inline functions ******* //

namespace invariant{
template <typename _Tp>
const ibex::IntervalVector& Face<_Tp>::get_position() const{
    return m_position;
}

template <typename _Tp>
const _Tp Face<_Tp>::get_position_typed() const{ // Copy of the face because of bug in multiprocessing using ppl (?)
    return m_position_typed;
}

template <typename _Tp>
const ibex::IntervalVector& Face<_Tp>::get_orientation() const{
    return m_orientation;
}

template <typename _Tp>
const ibex::IntervalVector &Face<_Tp>::get_normal() const{
    return m_normal;
}

template <typename _Tp>
const std::vector<Face<_Tp> *> &Face<_Tp>::get_neighbors() const{
    return m_neighbors;
}

template <typename _Tp>
std::ostream& operator<< (std::ostream& stream, const Face<_Tp>& f) {
    stream << f.get_position();
    return stream;
}

template <typename _Tp>
const bool Face<_Tp>::is_equal(const Face<_Tp>& f) const{
    if(m_position == f.get_position() && this->m_orientation==f.get_orientation())
        return true;
    else
        return false;
}

template <typename _Tp>
std::map<Maze<_Tp> *, Door<_Tp> *> Face<_Tp>::get_doors(){
    return m_doors;
}

template <typename _Tp>
Pave<_Tp>* Face<_Tp>::get_pave() const{
    return m_pave;
}

template <typename _Tp>
bool Face<_Tp>::is_border() const{
    return m_border;
}

template <typename _Tp>
bool Face<_Tp>::analyze_border(){
    if(m_neighbors.size() == 0)
        m_border = true;
    return m_border;
}

template <typename _Tp>
void Face<_Tp>::set_border(){
    m_border = true;
}

template <typename _Tp>
void Face<_Tp>::serialize(std::ofstream &binFile) const{}

template <typename _Tp>
void Face<_Tp>::deserialize(std::ifstream& binFile){}

/// ******************  Sepcialized ****************** ///

}

#include "face.tpp"

#endif // FACE_H
