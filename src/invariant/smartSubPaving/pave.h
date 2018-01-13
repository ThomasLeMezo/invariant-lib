#ifndef PAVE_H
#define PAVE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <map>
#include <fstream>
#include <utility>

#include "smartSubPaving.h"
#include "face.h"
#include "pave_node.h"

#include "room.h"
#include "maze.h"
#include "domain.h"

#include "../serialization/ibex_serialization.h"

namespace invariant {

template <typename _Tp> class Pave;
using PavePPL = Pave<Parma_Polyhedra_Library::C_Polyhedron>;
using PaveIBEX = Pave<ibex::IntervalVector>;

template <typename _Tp> class SmartSubPaving;
template <typename _Tp> class Room;
template <typename _Tp> class Face;
template <typename _Tp> class Maze;
template <typename _Tp> class Pave_node;

template <typename _Tp=ibex::IntervalVector>
class Pave
{
public:
    /**
     * @brief Construct a Pave with position and a SmartSubPaving
     * @param position of the Pave
     */
    Pave(const ibex::IntervalVector &position, SmartSubPaving<_Tp>* g);

    /**
     * @brief Construct an empty Pave link to a SmartSubPaving
     * @param g
     */
    Pave(SmartSubPaving<_Tp>* g);

    /**
     * @brief Construct an empty Pave
     */
    Pave():m_position(0), m_faces(0){}

    /**
     * @brief Pave destructor
     */
    ~Pave();

    /**
     * @brief Return the position of the Pave
     * @return an interval vector of the position
     */
    const ibex::IntervalVector &get_position() const;
    const ibex::IntervalVector get_position_copy() const;

    const _Tp &get_position_typed() const;
    const _Tp get_position_copy_typed() const;

    /**
     * @brief Return the array of an array of Faces of the Pave
     * @return A two arrays of pointer to the faces
     */
    const std::vector< std::array<Face<_Tp> *, 2>>& get_faces() const;

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
    const std::array<Face<_Tp>*, 2>& operator[](const std::size_t& i) const;

    /**
     * @brief Bisect the Pave
     * - add the result to the SmartSubPaving
     * - upate neighbors
     */
    bool bisect();

    bool bisect_step_one();

    void bisect_step_two();

    /**
     * @brief Return true if the Pave have to be bisected
     * (temporary : need to implement strategy of bisection)
     * @return
     */
    const bool request_bisection();

    /**
     * @brief Return the two child Paves after calling bisection
     * @return
     */
//    const std::array<Pave<_Tp> *, 2> &get_result_bisected();

    /**
     * @brief Return all the Faces of the Pave in a vector
     * @return
     */
    const std::vector<Face<_Tp> *> &get_faces_vector();

    /**
     * @brief Setter to the pave node associated with this node
     * @param pave_node
     */
    void set_pave_node(Pave_node<_Tp> *pave_node);

    /**
     * @brief Getter to the maze/room map
     * @return
     */
    std::map<Maze<_Tp> *, Room<_Tp> *> get_rooms() const;

    /**
     * @brief Add a new room to the map
     * @param maze
     * @param room
     */
    void add_room(Room<_Tp> *r);

    /**
     * @brief Return true if the pave contain an infinite boundary
     */
//    bool is_infinite() const;

    /**
     * @brief Return the dimension of the maze
     * @return
     */
    size_t &get_dim() const;

    /**
     * @brief Get the pave node associated to this Pave
     * @return
     */
    Pave_node<_Tp>* get_tree() const;

    /**
     * @brief Return true if this pave has one or more border faces (face with no neighbors)
     * @return
     */
    const bool &is_border() const;

    /**
     * @brief Analyze if this Pave is a border (see is_border)
     */
    void analyze_border();

    /**
     * @brief @brief Add all neighbors pave to the Pave list
     * @param pave_list
     */
    void get_neighbors_pave(std::vector<Pave<_Tp>*> pave_list);

    /**
     * @brief @brief Add all neighbors room to the Pave list
     * @param room_list
     * @param maze
     */
    void get_neighbors_room(Maze<_Tp> *maze, std::vector<Room<_Tp> *> &room_list);

    /**
     * @brief Set all the rooms attached to this pave to removed state
     */
    void set_removed_rooms();

    /**
     * @brief Get the dimension of the result of the intersection of a box
     * and the boundary of the pave
     * @param box
     * @return
     */
    int get_dim_inter_boundary(const ibex::IntervalVector &box);

    /**
     * @brief get_pave_children
     * @return
     */
    std::array<Pave<_Tp>*, 2>& get_pave_children() const;

    /**
     * @brief getBisection_axis
     * @return
     */
    size_t* get_bisection_axis() const;

private:
    /**
     * @brief compute typed position
     */
    void compute_typed_position();

private:

    /** Class Variable **/
    mutable ibex::IntervalVector                            m_position; // Pave position
    mutable _Tp                                             *m_position_typed; // Pave position
    mutable std::vector< std::array<Face<_Tp>*, 2>>         m_faces; // Faces of the Pave
    mutable std::vector<invariant::Face<_Tp> *>             m_faces_vector; // Faces of the Pave
    mutable SmartSubPaving<_Tp>*                            m_subpaving = nullptr;
    mutable Pave_node<_Tp>*                                 m_tree = nullptr;
    mutable size_t                                          m_dim = 0;
    std::map<Maze<_Tp>*, Room<_Tp>*>                        m_rooms;
//    bool                                                    m_infinite_pave = false;
    size_t                                                  m_serialization_id=0;
    bool                                                    m_border = false;

    std::array<Pave<_Tp>*, 2> *m_pave_children = nullptr;
    size_t *m_bisection_axis = nullptr;

    //    std::map<Maze*, Door*>                      m_initial_condition_door;
    //    std::map<Maze*, Door*>                      m_hybrid_door;
};

/**
     * @brief Overloading of the operator << for a Pave
     * @param stream
     * @param Pave
     * @return
     */
template<typename _Tp>
std::ostream& operator<< (std::ostream& stream, const Pave<_Tp>& p);

/**
     * @brief Overloading of the operator << for a vector of Paves
     * @param stream
     * @param l
     * @return
     */
template<typename _Tp>
std::ostream& operator<< (std::ostream& stream, const std::vector<Pave<_Tp>*> &l);
}

/// ***** Inline functions *****///
namespace invariant{

template <typename _Tp>
inline const ibex::IntervalVector& Pave<_Tp>::get_position() const{
    return m_position;
}

template <typename _Tp>
const _Tp& Pave<_Tp>::get_position_typed() const{
    return *m_position_typed;
}

template <typename _Tp>
inline const std::vector<std::array<Face<_Tp> *, 2> > &Pave<_Tp>::get_faces() const{
    return m_faces;
}

template <typename _Tp>
inline const size_t& Pave<_Tp>::get_serialization_id() const{
    return m_serialization_id;
}

template <typename _Tp>
inline void Pave<_Tp>::set_serialization_id(const size_t &value){
    m_serialization_id = value;
}

template<typename _Tp>
inline std::ostream& operator<< (std::ostream& stream, const Pave<_Tp>& p) {
    stream << p.get_position();
    return stream;
}

template <typename _Tp>
inline const std::array<Face<_Tp>*, 2>& Pave<_Tp>::operator[](const std::size_t& i) const{
    return m_faces[i];
}

//template <typename _Tp>
//inline const std::array<Pave<_Tp> *, 2>& Pave<_Tp>::get_result_bisected(){
//    return m_result_bisected;
//}

template <typename _Tp>
inline const std::vector<Face<_Tp> *> &Pave<_Tp>::get_faces_vector(){
    return m_faces_vector;
}

template <typename _Tp>
inline void Pave<_Tp>::set_pave_node(Pave_node<_Tp> *pave_node){
    m_tree = pave_node;
}

template <typename _Tp>
inline std::map<Maze<_Tp> *, Room<_Tp>*> Pave<_Tp>::get_rooms() const{
    return m_rooms;
}

//template <typename _Tp>
//inline bool Pave<_Tp>::is_infinite() const{
//    return m_infinite_pave;
//}

template <typename _Tp>
size_t& Pave<_Tp>::get_dim() const{
    return m_dim;
}

template <typename _Tp>
inline Pave_node<_Tp>* Pave<_Tp>::get_tree() const{
    return m_tree;
}

template <typename _Tp>
inline const bool& Pave<_Tp>::is_border() const{
    return m_border;
}

/// ******************  Sepcialized ****************** ///

}

#include "pave.tpp"

#endif // PAVE_H
