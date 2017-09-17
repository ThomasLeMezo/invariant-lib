#ifndef ROOM_H
#define ROOM_H

#include <ibex/ibex_IntervalVector.h>
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
     * @brief Set all the private output doors to empty
     */
    void set_empty_private_output();

    /**
     * @brief Set all the private input doors to empty
     */
    void set_empty_private_input();

    /**
     * @brief Set all the private output doors to full
     */
    void set_full_private_output();

    /**
     * @brief Set all the private input doors to full
     */
    void set_full_private_input();

    /**
     * @brief Set all input and output doors to empty
     */
    void set_empty_private();

    /**
     * @brief Set all input and output private doors to full
     */
    void set_full();

    /**
     * @brief Set full the room according to vector field possibilities
     * @return
     */
    void set_full_possible();

    /**
     * @brief Function call after Pave bisection to update Room & Doors
     */
    void bisect();

    /**
     * @brief Test if this room is in the deque of the maze
     * @return
     */
    bool is_in_deque();

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
     * @brief Return true if this Room is empty
     * (true if all doors are empty)
     * @return
     */
    bool is_empty();

    /**
     * @brief Return true if this Room is full
     * (true if all doors are full)
     * @return
     */
    bool is_full();

    /**
     * @brief Getter to the vector fields
     * @return
     */
    const std::vector<ibex::IntervalVector>& get_vector_fields() const;

    /**
     * @brief Getter to one of the the vector fields
     * @return
     */
    const ibex::IntervalVector get_one_vector_fields(int n_vf) const;

    /**
     * @brief Getter to one of the the vector fields zero evaluation
     * @param n_vf
     * @return
     */
    const bool get_one_vector_fields_zero(int n_vf) const;

    /**
     * @brief Lock the pave to other contractions
     */
    void lock_contraction();

    /**
     * @brief Unlock the pave to other contractions
     */
    void unlock_contraction();

    /**
     * @brief Synchronize the public doors with the private one
     */
    void synchronize();

    /**
     * @brief Get if this room is removed (i.e. if it is active)
     * @return
     */
    bool is_removed() const;

    /**
     * @brief Set this room to removed state
     */
    void set_removed();

    /**
     * @brief Get the vector of bool telling if zero belongs to the vector field
     * @return
     */
    std::vector<bool> get_vector_fields_zero();

    /**
     * @brief Get the box convex hull of the polygon
     * @return
     */
    const ibex::IntervalVector get_hull() const;

    /**
     * @brief Get the complementary box convex hull of the polygon
     * @return
     */
    const ibex::IntervalVector get_hull_complementary();


protected:
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
     * @brief Test the doors according to the vector field orientation and save the result in each doors
     */
    void eval_vector_field_possibility();

    /**
     * @brief Basic contraction between [in] and [out] according to a [vect].
     * @param in
     * @param out
     * @param vect
     */
    void contract_flow(ibex::IntervalVector &in, ibex::IntervalVector &out, const ibex::IntervalVector &vect);

    /**
     * @brief Temp
     * @param iv
     * @return
     */
//    bool is_degenerated(const ibex::IntervalVector& iv);

    /**
     * @brief Get door contraction in case of sliding mode (IN & OUT)
     * @param n_vf
     * @param face_in
     * @param sens_in
     * @param out_return
     * @param in_return
     */
    void contract_sliding_mode(int n_vf, int face_in, int sens_in, ibex::IntervalVector &out_return, ibex::IntervalVector &in_return);

public:
    /**
     * @brief Debug function to get private doors
     * @param position
     */
    bool get_private_doors_info(std::string message="", bool cout_message=true);

    /**
     * @brief Get the number of contractions call
     * @return
     */
    int get_nb_contractions() const;

protected:
    Pave*   m_pave = NULL; // pointer to the associated face
    Maze*   m_maze = NULL; // pointer to the associated maze
    std::vector<ibex::IntervalVector> m_vector_fields; // Vector field of the Room
    std::vector<bool>    m_vector_field_zero;

    mutable bool    m_empty = false;
    mutable bool    m_full = false;

    bool m_full_first_eval = true;
    bool m_empty_first_eval = false;

    bool    m_first_contract = true; // Use to contract according to the vector_field
    omp_lock_t   m_lock_contraction; // Lock the Room when contractor function is called
    omp_lock_t   m_lock_deque; // Lock in_deque variable access

    bool    m_in_deque = false;
    bool    m_removed = false;

    int     m_nb_contract = 0;

    bool m_debug_room = false;
    int m_time_debug = 0;
};
}

namespace invariant {
std::ostream& operator<< (std::ostream& stream, const Room& r);

int get_nb_dim_flat(const ibex::IntervalVector &iv);

inline Pave* Room::get_pave() const{
    return m_pave;
}

inline Maze* Room::get_maze() const{
    return m_maze;
}

inline bool Room::is_in_deque(){
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
}

inline const std::vector<ibex::IntervalVector>& Room::get_vector_fields() const{
    return m_vector_fields;
}

inline void Room::lock_contraction(){
    omp_set_lock(&m_lock_contraction);
}

inline void Room::unlock_contraction(){
    omp_unset_lock(&m_lock_contraction);
}

inline bool Room::is_removed() const{
    return m_removed;
}

inline int Room::get_nb_contractions() const{
    return m_nb_contract;
}

inline std::vector<bool> Room::get_vector_fields_zero(){
    return m_vector_field_zero;
}

}

#endif // ROOM_H
