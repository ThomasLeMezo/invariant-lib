#ifndef ROOM_H
#define ROOM_H

#include <ibex.h>
#include "pave.h"
#include "maze.h"
#include <omp.h>

namespace invariant {

class Pave; // declared only for friendship
class Maze; // declared only for friendship
class Room
{
public:
    /**
     * @brief Room constructor
     * @param p
     * @param f_vect
     */
    Room(Pave *p, Maze *m, std::vector<ibex::Function *> f_vect);

    /**
     * @brief Room destructor
     */
    ~Room();

    /**
     * @brief contract the pave (continuity + consistency)
     */
    void contract();

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
     * @brief Test if this room is in th deque of the maze
     * @return
     */
    bool is_in_deque() const;

private:
    /**
     * @brief Contract doors according to the neighbors
     */
    void contract_continuity();

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

    /**
     * @brief Synchronize all the private doors to the public one
     */
    void synchronize_doors();

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
}

#endif // ROOM_H
