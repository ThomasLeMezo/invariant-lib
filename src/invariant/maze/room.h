#ifndef ROOM_H
#define ROOM_H

#include <ibex.h>
#include "pave.h"
#include "maze.h"
#include "../definition/dynamics.h"
#include <omp.h>
#include "face.h"

namespace invariant {

class Pave; // declared only for friendship
class Maze; // declared only for friendship
class Dynamics; // declared only for friendship
class Face; // declared only for friendship
class Room
{
public:
    /**
     * @brief Room constructor
     * @param p
     * @param f_vect
     */
    Room(Pave *p, Maze *m, Dynamics *dynamics);

    /**
     * @brief Room destructor
     */
    ~Room();

    /**
     * @brief contract the pave (continuity + consistency)
     */
    bool contract();

    /**
     * @brief Getter to the associated Pave
     * @return
     */
    Pave* get_pave() const;

    /**
     * @brief Getter to the associated Maze
     * @return
     */
    Maze* get_maze() const;

    /**
     * @brief Set all the output doors to empty
     * Request synchronization (omp lock)
     */
    void set_empty_output();

    /**
     * @brief Set all the input doors to empty
     * Request synchronization (omp lock)
     */
    void set_empty_private_input();

    /**
     * @brief Set all input and output doors to empty
     * Request synchronization (omp lock)
     */
    void set_empty();

    /**
     * @brief Function call after Pave bisection to update Room & Doors
     */
    void bisect();

    /**
     * @brief Test if this room is in the deque of the maze
     * @return
     */
    bool is_in_deque() const;

    /**
     * @brief Declare this room in the deque of the maze
     */
    void set_in_queue();

    /**
     * @brief Synchronize all the private doors to the public one
     */
    void synchronize_doors();

    /**
     * @brief Return a list of neighbor Rooms that need an update according after contraction
     * @param list_rooms
     */
    void analyze_change(std::vector<Room *> &list_rooms);

    /**
     * @brief Reset deque state to false
     */
    void reset_deque();

    /**
     * @brief Test if this Room need a bisection
     *  answer false if empty
     */
    bool request_bisection();

    /**
     * @brief Return if this Room is empty
     * (true if all doors are empty)
     * @return
     */
    bool is_empty();

    /**
     * @brief Getter to the vector fields
     * @return
     */
    const std::vector<ibex::IntervalVector>& get_vector_fields() const;


private:
    /**
     * @brief Contract doors according to the neighbors
     */
    bool contract_continuity();

    /**
     * @brief Contract doors according to the vector field flow
     */
    void contract_consistency();

    /**
     * @brief Contract the doors according to the vector field orientation
     */
    void contract_vector_field();

    /**
     * @brief Basic contraction between [in] and [out] according to a [vect].
     * @param in
     * @param out
     * @param vect
     */
    void contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect);

private:
    Pave*   m_pave = NULL; // pointer to the associated face
    Maze*   m_maze = NULL; // pointer to the associated maze
    std::vector<ibex::IntervalVector> m_vector_fields; // Vector field of the Room

    bool    m_empty = false;
    bool    m_first_contract = true; // Use to contract according to the vector_field
    omp_lock_t   m_lock_contraction; // Lock the Room when contractor function is called
    mutable omp_lock_t   m_lock_deque; // Lock in_deque variable access

    bool    m_in_deque = false;

};
}

namespace invariant {
inline Pave* Room::get_pave() const{
    return m_pave;
}

inline Maze* Room::get_maze() const{
    return m_maze;
}

inline bool Room::is_in_deque() const{
    bool result;
    omp_set_lock(&m_lock_deque);
    result = m_in_deque;
    omp_unset_lock(&m_lock_deque);
    return result;
}

inline void Room::set_in_queue(){
    omp_set_lock(&m_lock_deque);
    m_in_deque = true;
    omp_unset_lock(&m_lock_deque);
}

inline void Room::reset_deque(){
    omp_set_lock(&m_lock_deque);
    m_in_deque = false;
    omp_unset_lock(&m_lock_deque);
    omp_unset_lock(&m_lock_contraction);
}

inline const std::vector<ibex::IntervalVector>& Room::get_vector_fields() const{
    return m_vector_fields;
}
}

#endif // ROOM_H
