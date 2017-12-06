#ifndef PAVE_H
#define PAVE_H

#include <ibex/ibex_IntervalVector.h>

#include <map>
#include <fstream>
#include <utility>

#include "smartSubPaving.h"
#include "face.h"
#include "pave_node.h"

#include "../maze/room.h"
#include "../maze/maze.h"
#include "../definition/domain.h"

#include "../serialization/ibex_serialization.h"

namespace invariant {

template <typename _Tp, typename _V> class Pave;
using PavePPL = Pave<Parma_Polyhedra_Library::C_Polyhedron, Parma_Polyhedra_Library::Generator_System>;
using PaveIBEX = Pave<ibex::IntervalVector, ibex::IntervalVector>;

template <typename _Tp, typename _V> class SmartSubPaving;
template <typename _Tp, typename _V> class Room;
template <typename _Tp, typename _V> class Face;
template <typename _Tp, typename _V> class Maze;
template <typename _Tp, typename _V> class Pave_node;

template <typename _Tp=ibex::IntervalVector, typename _V=ibex::IntervalVector>
class Pave
{
public:
    /**
     * @brief Construct a Pave with position and a SmartSubPaving
     * @param position of the Pave
     */
    Pave(const ibex::IntervalVector &position, SmartSubPaving<_Tp, _V>* g);

    /**
     * @brief Construct an empty Pave link to a SmartSubPaving
     * @param g
     */
    Pave(SmartSubPaving<_Tp, _V>* g);

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

    const _Tp get_position_typed() const;
    const _Tp get_position_copy_typed() const;

    /**
     * @brief Return the array of an array of Faces of the Pave
     * @return A two arrays of pointer to the faces
     */
    const std::vector< std::array<Face<_Tp, _V> *, 2>>& get_faces() const;

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
    const std::array<Face<_Tp, _V>*, 2>& operator[](const std::size_t& i) const;

    /**
     * @brief Bisect the Pave
     * - add the result to the SmartSubPaving
     * - upate neighbors
     */
    void bisect();

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
    const std::array<Pave<_Tp, _V> *, 2> &get_result_bisected();

    /**
     * @brief Return all the Faces of the Pave in a vector
     * @return
     */
    const std::vector<Face<_Tp, _V> *> &get_faces_vector();

    /**
     * @brief Setter to the pave node associated with this node
     * @param pave_node
     */
    void set_pave_node(Pave_node<_Tp, _V> *pave_node);

    /**
     * @brief Getter to the maze/room map
     * @return
     */
    std::map<Maze<_Tp, _V> *, Room<_Tp, _V> *> get_rooms() const;

    /**
     * @brief Add a new room to the map
     * @param maze
     * @param room
     */
    void add_room(Room<_Tp, _V> *r);

    /**
     * @brief Return true if the pave contain an infinite boundary
     */
    bool is_infinite() const;

    /**
     * @brief Return the dimension of the maze
     * @return
     */
    size_t get_dim() const;

    /**
     * @brief Get the pave node associated to this Pave
     * @return
     */
    Pave_node<_Tp, _V>* get_tree() const;

    /**
     * @brief Return true if this pave has one or more border faces (face with no neighbors)
     * @return
     */
    bool is_border() const;

    /**
     * @brief Analyze if this Pave is a border (see is_border)
     */
    void analyze_border();

    /**
     * @brief @brief Add all neighbors pave to the Pave list
     * @param pave_list
     */
    void get_neighbors_pave(std::vector<Pave<_Tp, _V>*> pave_list);

    /**
     * @brief @brief Add all neighbors room to the Pave list
     * @param room_list
     * @param maze
     */
    void get_neighbors_room(Maze<_Tp, _V> *maze, std::vector<Room<_Tp, _V> *> &room_list);

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

private:

    /** Class Variable **/
    mutable ibex::IntervalVector                            m_position; // Pave position
    mutable std::vector< std::array<Face<_Tp, _V>*, 2>>         m_faces; // Faces of the Pave
    mutable std::vector<invariant::Face<_Tp, _V> *>             m_faces_vector; // Faces of the Pave
    mutable SmartSubPaving<_Tp, _V>*                            m_subpaving = NULL;
    mutable Pave_node<_Tp, _V>*                                 m_tree = NULL;
    mutable size_t                                          m_dim = 0;
    std::array<Pave<_Tp, _V>*, 2>                               m_result_bisected;
    std::map<Maze<_Tp, _V>*, Room<_Tp, _V>*>                        m_rooms;
    bool                                                    m_infinite_pave = false;
    size_t                                                  m_serialization_id=0;
    bool                                                    m_border = false;

    //    std::map<Maze*, Door*>                      m_initial_condition_door;
    //    std::map<Maze*, Door*>                      m_hybrid_door;
};

/**
     * @brief Overloading of the operator << for a Pave
     * @param stream
     * @param Pave
     * @return
     */
template<typename _Tp, typename _V>
std::ostream& operator<< (std::ostream& stream, const Pave<_Tp, _V>& p);

/**
     * @brief Overloading of the operator << for a vector of Paves
     * @param stream
     * @param l
     * @return
     */
template<typename _Tp, typename _V>
std::ostream& operator<< (std::ostream& stream, const std::vector<Pave<_Tp, _V>*> &l);
}

/// ***** Inline functions *****///
namespace invariant{

template <typename _Tp, typename _V>
inline const ibex::IntervalVector& Pave<_Tp, _V>::get_position() const{
    return m_position;
}

template <typename _Tp, typename _V>
inline const std::vector<std::array<Face<_Tp, _V> *, 2> > &Pave<_Tp, _V>::get_faces() const{
    return m_faces;
}

template <typename _Tp, typename _V>
inline const size_t& Pave<_Tp, _V>::get_serialization_id() const{
    return m_serialization_id;
}

template <typename _Tp, typename _V>
inline void Pave<_Tp, _V>::set_serialization_id(const size_t &value){
    m_serialization_id = value;
}

template<typename _Tp, typename _V>
inline std::ostream& operator<< (std::ostream& stream, const Pave<_Tp, _V>& p) {
    stream << p.get_position();
    return stream;
}

template <typename _Tp, typename _V>
inline const std::array<Face<_Tp, _V>*, 2>& Pave<_Tp, _V>::operator[](const std::size_t& i) const{
    return m_faces[i];
}

template <typename _Tp, typename _V>
inline const std::array<Pave<_Tp, _V> *, 2>& Pave<_Tp, _V>::get_result_bisected(){
    return m_result_bisected;
}

template <typename _Tp, typename _V>
inline const std::vector<Face<_Tp, _V> *> &Pave<_Tp, _V>::get_faces_vector(){
    return m_faces_vector;
}

template <typename _Tp, typename _V>
inline void Pave<_Tp, _V>::set_pave_node(Pave_node<_Tp, _V> *pave_node){
    m_tree = pave_node;
}

template <typename _Tp, typename _V>
inline std::map<Maze<_Tp, _V> *, Room<_Tp, _V>*> Pave<_Tp, _V>::get_rooms() const{
    return m_rooms;
}

template <typename _Tp, typename _V>
inline bool Pave<_Tp, _V>::is_infinite() const{
    return m_infinite_pave;
}

template <typename _Tp, typename _V>
size_t Pave<_Tp, _V>::get_dim() const{
    return m_dim;
}

template <typename _Tp, typename _V>
inline Pave_node<_Tp, _V>* Pave<_Tp, _V>::get_tree() const{
    return m_tree;
}

template <typename _Tp, typename _V>
inline bool Pave<_Tp, _V>::is_border() const{
    return m_border;
}

/// ******************  Sepcialized ****************** ///

}

#include "pave.tpp"

#endif // PAVE_H
