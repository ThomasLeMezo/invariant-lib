#ifndef PAVE_H
#define PAVE_H

#include <ibex/ibex_IntervalVector.h>

#include <map>
#include <fstream>
#include "room.h"
#include "face.h"
#include "smartSubPaving.h"
#include "maze.h"


namespace invariant {

class Face; // declared only for friendship
class SmartSubPaving; // declared only for friendship
class Maze; // declared only for friendship
class Room; // declared only for friendship
class Pave_node; // declared only for friendship

class Pave
{
public:
    /**
     * @brief Construct a Pave with position and a SmartSubPaving
     * @param position of the Pave
     */
    Pave(const ibex::IntervalVector &position, SmartSubPaving* g);

    /**
     * @brief Construct an empty Pave link to a SmartSubPaving
     * @param g
     */
    Pave(SmartSubPaving* g);

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
    const std::array<Pave *, 2> &get_result_bisected();

    /**
     * @brief Return all the Faces of the Pave in a vector
     * @return
     */
    const std::vector<Face *> &get_faces_vector();

    /**
     * @brief Setter to the pave node associated with this node
     * @param pave_node
     */
    void set_pave_node(Pave_node *pave_node);

    /**
     * @brief Getter to the maze/room map
     * @return
     */
    std::map<Maze *, Room *> get_rooms() const;

    /**
     * @brief Add a new room to the map
     * @param maze
     * @param room
     */
    void add_room(Room *r);

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
    Pave_node* get_tree() const;

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
    void get_neighbors_pave(std::vector<Pave*> pave_list);

    /**
     * @brief @brief Add all neighbors room to the Pave list
     * @param room_list
     * @param maze
     */
    void get_neighbors_room(Maze *maze, std::vector<Room *> &room_list);

    /**
     * @brief Set all the rooms attached to this pave to removed state
     */
    void set_removed_rooms();

private:

    /** Class Variable **/
    mutable ibex::IntervalVector                m_position; // Pave position
    mutable std::vector< std::array<Face*, 2>>  m_faces; // Faces of the Pave
    mutable std::vector<Face *>                 m_faces_vector; // Faces of the Pave
    mutable SmartSubPaving*                              m_subpaving = NULL;
    mutable Pave_node*                          m_tree = NULL;
    mutable size_t                              m_dim = 0;
    std::array<Pave*, 2>                        m_result_bisected;
    std::map<Maze*, Room*>                      m_rooms;
    bool                                        m_infinite_pave = false;
    size_t                                      m_serialization_id=0;
    bool                                        m_border = false;
};

    /**
     * @brief Overloading of the operator << for a Pave
     * @param stream
     * @param Pave
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const Pave& p);

    /**
     * @brief Overloading of the operator << for a vector of Paves
     * @param stream
     * @param l
     * @return
     */
    std::ostream& operator<< (std::ostream& stream, const std::vector<Pave*> &l);
}

/// ***** Inline functions *****///
namespace invariant{

inline const ibex::IntervalVector& Pave::get_position() const{
    return m_position;
}

inline const std::vector<std::array<Face *, 2> > &Pave::get_faces() const{
    return m_faces;
}

inline const size_t& Pave::get_serialization_id() const{
    return m_serialization_id;
}

inline void Pave::set_serialization_id(const size_t &value){
    m_serialization_id = value;
}

inline std::ostream& operator<< (std::ostream& stream, const Pave& p) {
    stream << p.get_position();
    return stream;
}

inline const std::array<Face*, 2>& Pave::operator[](const std::size_t& i) const{
    return m_faces[i];
}

inline const std::array<Pave *, 2>& Pave::get_result_bisected(){
    return m_result_bisected;
}

inline const std::vector<Face *> &Pave::get_faces_vector(){
    return m_faces_vector;
}

inline void Pave::set_pave_node(Pave_node *pave_node){
    m_tree = pave_node;
}

inline std::map<Maze *, Room*> Pave::get_rooms() const{
    return m_rooms;
}

inline bool Pave::is_infinite() const{
    return m_infinite_pave;
}

inline size_t Pave::get_dim() const{
    return m_dim;
}

inline Pave_node* Pave::get_tree() const{
    return m_tree;
}

inline bool Pave::is_border() const{
    return m_border;
}
}

#endif // PAVE_H
