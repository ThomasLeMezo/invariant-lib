#ifndef FUNCTION_TOOLS
#define FUNCTION_TOOLS
namespace invariant {
template <typename _Tp>
_Tp get_empty_door_container(int dim);
}
#endif


#ifndef ROOM_H
#define ROOM_H

#include <ibex_IntervalVector.h>
#include <ibex_IntervalMatrix.h>
#include <ibex_Function.h>
#include <ibex_CtcFwdBwd.h>

#include <ppl.hh>

#include <omp.h>

#include "dynamics.h"
#include "domain.h"
#include "pave.h"
#include "maze.h"
#include "door.h"
#include "face.h"
#include "resultstorage.h"

namespace ppl=Parma_Polyhedra_Library;

namespace invariant {

template <typename _Tp> class Room;
using RoomPPL = Room<Parma_Polyhedra_Library::C_Polyhedron>;
using RoomIBEX = Room<ibex::IntervalVector>;

class Dynamics; // declared only for friendship
template <typename _Tp> class Pave; // declared only for friendship
template <typename _Tp> class Maze;
template <typename _Tp> class Face;
template <typename _Tp> class Door;
template <typename _Tp> class Domain;

template <typename _Tp=ibex::IntervalVector>
class Room
{
public:
    /**
     * @brief Room constructor
     * @param p
     * @param f_vect
     */
    Room(Pave<_Tp> *p, Maze<_Tp> *m, Dynamics *dynamics);

    /**
     * @brief Room destructor
     */
    ~Room();

    /**
     * @brief Room
     * @param r
     */
//    Room(const Room<_Tp> &r);

    /**
     * @brief reset
     */
    void reset();

    /**
     * @brief contract the pave (continuity + consistency)
     */
    bool contract();

    /**
     * @brief Getter to the associated Pave
     * @return
     */
    Pave<_Tp>* get_pave() const;

    /**
     * @brief Getter to the associated Maze
     * @return
     */
    Maze<_Tp>* get_maze() const;

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
    void set_full_private();

    /**
     * @brief set full private but taking in account father
     */
    void set_full_private_with_father();

    /**
     * @brief Set full the room according to vector field possibilities
     * @return
     */
    void set_full_possible();

    /**
     * @brief Test if this room is in the deque of the maze
     * @return
     */
    bool is_in_deque();

    /**
     * @brief Declare this room in the deque of the maze
     */
    bool set_in_queue();

    /**
     * @brief Return a list of neighbor Rooms that need an update according after contraction
     * @param list_rooms
     */
    void analyze_change(std::vector<Room<_Tp> *> &list_rooms) const;

    /**
     * @brief Get the list of all the not removed neighbors of this room
     * @param list_rooms
     */
    void get_all_active_neighbors(std::vector<Room<_Tp> *> &list_rooms) const;

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
    const bool is_empty();
    const bool is_empty_private();

    /**
     * @brief Return true if this Room is full
     * (true if all doors are full)
     * @return
     */
    const bool is_full();
    const bool is_full_private();

    /**
     * @brief Return true if the Union of outer & inner for each doors is full
     * @return
     */
    bool is_full_union() const;

    /**
     * @brief Getter to the vector fields
     * @return
     */
    std::vector<ibex::IntervalVector> get_vector_fields() const;
    std::vector<ibex::IntervalMatrix> get_vector_fields_d() const;

    /**
     * @brief Getter to one of the the vector fields
     * @return
     */
    ibex::IntervalVector get_one_vector_fields(int n_vf) const;
    const _Tp& get_one_vector_fields_typed_fwd(int n_vf) const;
    const _Tp& get_one_vector_fields_typed_bwd(int n_vf) const;

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
     * @brief reset update neighbors
     */
//    void reset_update_neighbors();

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
    const _Tp get_hull_typed() const;

    /**
     * @brief Get the complementary box convex hull of the polygon
     * @return
     */
    const ibex::IntervalVector get_hull_complementary();

    /**
     * @brief Return true if one vector field has a zero on one of its coordinate
     * @return
     */
    bool get_contain_zero_coordinate() const;

    /**
     * @brief Return true if one vector field contains zero value
     * @return
     */
    bool get_contain_zero() const;

    /**
     * @brief compute the vector field typed (in the constructor of Room)
     */
    void compute_vector_field_typed();

    /**
     * @brief Set the union of all the doors and the hull
     * @param hull
     */
    void set_union_hull(const _Tp &hull);

    /**
     * @brief operator &=
     * @param r1
     * @param r2
     * @return
     */
    friend Room<_Tp>& operator&=(Room<_Tp>& r1, const Room<_Tp>& r2){
        for(Face<_Tp> *f:r1.get_pave()->get_faces_vector()){
            Door<_Tp> *d1 = f->get_doors()[r1.get_maze()];
            Door<_Tp> *d2 = f->get_doors()[r2.get_maze()];
            *d1 &= *d2;
        }
        return r1;
    }
    /**
     * @brief operator |=
     * @param r1
     * @param r2
     * @return
     */
    friend Room<_Tp>& operator|=(Room<_Tp>& r1, const Room<_Tp>& r2){
        for(Face<_Tp> *f:r1.get_pave()->get_faces_vector()){
            Door<_Tp> *d1 = f->get_doors()[r1.get_maze()];
            Door<_Tp> *d2 = f->get_doors()[r2.get_maze()];
            *d1 |= *d2;
        }
        return r1;
    }

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
     * and Test the doors according to the vector field orientation and save the result in each doors
     */
    void contract_vector_field();

    /**
     * @brief contract according to vector field (similar to contract_vector_field but used only when vector field possibilities
     * has been already compute)
     */
    void contract_according_to_vector_field();

    /**
     * @brief Basic contraction between [in] and [out] according to a [vect].
     * @param in
     * @param out
     * @param vect
     */
    void contract_flow(_Tp &in, _Tp &out, const _Tp &vect, const DYNAMICS_SENS &sens);
//    void contract_flow(ppl::C_Polyhedron &in, ppl::C_Polyhedron &out, const ibex::IntervalVector &vect);

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
    void contract_sliding_mode(int n_vf, int face_in, int sens_in, _Tp &out_return, _Tp &in_return);

    /**
     * @brief compute_sliding_mode
     * @param n_vf
     * @param out_results
     */
    void compute_sliding_mode(const int n_vf, ResultStorage<_Tp> &result_storage);

    /**
     * @brief compute_standard_mode
     * @param n_vf
     * @param out_results
     */
    void compute_standard_mode(const int n_vf, ResultStorage<_Tp> &result_storage);

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
    size_t get_nb_contractions() const;

    /**
     * @brief get vector fields typed
     * @return
     */
    const std::vector<_Tp>& get_vector_fields_typed_fwd() const;
    const std::vector<_Tp>& get_vector_fields_typed_bwd() const;

    /**
     * @brief recompute_vector_field
     */
    void compute_vector_field();

    /**
     * @brief is initial door input/output
     * @return
     */
    bool is_initial_door_input() const;
    bool is_initial_door_output() const;

    /**
     * @brief set_initial_condition
     */
    void set_initial_door_input(const _Tp &door);
    void set_initial_door_output(const _Tp &door);

    const _Tp& get_initial_door_input() const;
    const _Tp& get_initial_door_output() const;

    void set_full_initial_door_input();
    void set_full_initial_door_output();

    void set_father_hull(const _Tp &hull);
    bool is_father_hull() const;
    const _Tp& get_father_hull() const;

    bool is_first_contract() const;
    bool is_contract_vector_field() const;

    /**
     * @brief reset first contract
     */
    void reset_first_contract();

    void reset_init_door_input();
    void reset_init_door_output();

protected:
    Pave<_Tp>*   m_pave = nullptr; // pointer to the associated face
    Maze<_Tp>*   m_maze = nullptr; // pointer to the associated maze
    std::vector<ibex::IntervalVector> m_vector_fields; // Vector field of the Room
    std::vector<ibex::IntervalMatrix> m_vector_fields_d1; // Vector field of the Room

    std::vector<_Tp> m_vector_fields_typed_fwd; // Typed Vector field of the Room
    std::vector<_Tp> m_vector_fields_typed_bwd; // Typed Vector field of the Room

    std::vector<bool>    m_vector_field_zero;
    bool            m_contain_zero_coordinate = false;
    bool            m_contain_zero = false; // if a zero in one of the list vect

    mutable bool    m_empty = false;
    mutable bool    m_full = false;

    bool m_full_first_eval = true;
    bool m_empty_first_eval = true;

    bool    m_first_contract = true; // First contract
    bool    m_contract_vector_field = true; // Use to contract according to the vector_field
    omp_lock_t   m_lock_contraction; // Lock the Room when contractor function is called
    omp_lock_t   m_lock_deque; // Lock in_deque variable access
    omp_lock_t   m_lock_vector_field; // Lock m_vector_fields variable access

    bool    m_in_deque = false;
    bool    m_removed = false;

    // Initial condition
    bool    m_is_initial_door_input = false;
    bool    m_is_initial_door_output = false;
    _Tp* m_initial_door_input = nullptr;
    _Tp* m_initial_door_output = nullptr;

    // Valid hull
    bool m_is_father_hull = false;
    _Tp* m_father_hull = nullptr;

    // To Be Removed
    size_t     m_nb_contract = 0;

//    ibex::Function *m_contract_function = nullptr;
//    ibex::CtcFwdBwd *m_ctc = nullptr;

    // For PPL
//    std::vector<std::vector<ppl::Generator>> *m_ray_vector_field;
//    std::vector<std::vector<ppl::Generator>> *m_ray_vector_field_backward;
};
}

namespace invariant {

inline Parma_Polyhedra_Library::C_Polyhedron& operator&=(Parma_Polyhedra_Library::C_Polyhedron& p1, const Parma_Polyhedra_Library::C_Polyhedron& p2){
    p1.intersection_assign(p2);
    return p1;
}

inline Parma_Polyhedra_Library::C_Polyhedron& operator|=(Parma_Polyhedra_Library::C_Polyhedron& p1, const Parma_Polyhedra_Library::C_Polyhedron& p2){
    p1.poly_hull_assign(p2);
    return p1;
}

inline Parma_Polyhedra_Library::C_Polyhedron operator&(const Parma_Polyhedra_Library::C_Polyhedron& p1, const Parma_Polyhedra_Library::C_Polyhedron& p2){
    Parma_Polyhedra_Library::C_Polyhedron p_return(p1);
    p_return.intersection_assign(p2);
    return p_return;
}

inline Parma_Polyhedra_Library::C_Polyhedron operator|(const Parma_Polyhedra_Library::C_Polyhedron& p1, const Parma_Polyhedra_Library::C_Polyhedron& p2){
    Parma_Polyhedra_Library::C_Polyhedron p_return(p1);
    p_return.poly_hull_assign(p2);
    return p_return;
}

ibex::IntervalVector convert_iv(const ibex::IntervalVector &iv);

ibex::IntervalVector convert_iv(const ppl::C_Polyhedron &p);

///
/// \brief Get the difference Hull a\b
/// \param a
/// \param b
/// \return
///
template <typename _Tp>
_Tp get_diff_hull(const _Tp &a, const _Tp &b);

template <typename _Tp>
void set_empty(_Tp &T);

int get_nb_dim_flat(const ibex::IntervalVector &iv);
int get_nb_dim_flat(const ppl::C_Polyhedron &p);

template<typename _Tp>
std::ostream& operator<< (std::ostream& stream, const Room<_Tp>& r);

template<typename _Tp>
_Tp convert_vec_field(const ibex::IntervalVector &vect);

template <typename _Tp>
inline Pave<_Tp>* Room<_Tp>::get_pave() const{
    return m_pave;
}

template <typename _Tp>
inline void Room<_Tp>::reset_first_contract(){
    m_first_contract = true;
    m_full=true;
    m_full_first_eval=true;
    m_empty=false;
    m_empty_first_eval=true;
    reset_init_door_input();
    reset_init_door_output();
}

template <typename _Tp>
inline void Room<_Tp>::reset_init_door_input(){
    if(m_is_initial_door_input){
        delete(m_initial_door_input);
        m_initial_door_input = nullptr;
        m_is_initial_door_input = false;
    }
}

template <typename _Tp>
inline void Room<_Tp>::reset_init_door_output(){
    if(m_is_initial_door_output){
        delete(m_initial_door_output);
        m_initial_door_output = nullptr;
        m_is_initial_door_output = false;
    }
}

template <typename _Tp>
inline bool Room<_Tp>::is_initial_door_input() const{
    return m_is_initial_door_input;
}

template <typename _Tp>
inline bool Room<_Tp>::is_initial_door_output() const{
    return m_is_initial_door_output;
}

template <typename _Tp>
inline void Room<_Tp>::set_initial_door_input(const _Tp &door){
    if(!m_is_initial_door_output || !m_is_initial_door_input)
        m_maze->add_initial_room(this);

    m_is_initial_door_input = true;
    if(m_initial_door_input == nullptr){
        m_initial_door_input = new _Tp(door);
    }
    else{
        *m_initial_door_input = door;
    }
}

template <typename _Tp>
inline void Room<_Tp>::set_initial_door_output(const _Tp &door){
    if(!m_is_initial_door_output || !m_is_initial_door_input)
        m_maze->add_initial_room(this);

    m_is_initial_door_output = true;
    if(m_initial_door_output == nullptr){
        m_initial_door_output = new _Tp(door);
    }
    else{
        *m_initial_door_output = door;
    }
}

template <typename _Tp>
inline const _Tp& Room<_Tp>::get_initial_door_input() const{
    return *m_initial_door_input;
}

template <typename _Tp>
inline const _Tp& Room<_Tp>::get_initial_door_output() const{
    return *m_initial_door_output;
}

template <typename _Tp>
inline void Room<_Tp>::set_full_initial_door_input(){
    if(!m_is_initial_door_output || !m_is_initial_door_input)
        m_maze->add_initial_room(this);

    m_is_initial_door_input = true;
    if(m_initial_door_input == nullptr){
        m_initial_door_input = new _Tp(m_pave->get_position_typed());
    }
    else{
        *m_initial_door_input = m_pave->get_position_typed();
    }
}

template <typename _Tp>
inline void Room<_Tp>::set_full_initial_door_output(){
    if(!m_is_initial_door_output || !m_is_initial_door_input)
        m_maze->add_initial_room(this);

    m_is_initial_door_output = true;
    if(m_initial_door_output == nullptr){
        m_initial_door_output = new _Tp(m_pave->get_position_typed());
    }
    else{
        *m_initial_door_output = m_pave->get_position_typed();
    }
}

template <typename _Tp>
inline void Room<_Tp>::set_father_hull(const _Tp &hull){
    m_is_father_hull = true;
    if(m_father_hull == nullptr){
        m_father_hull = new _Tp(hull);
    }
    else{
        *m_father_hull = hull;
    }
}

template <typename _Tp>
inline bool Room<_Tp>::is_father_hull() const{
    return m_is_father_hull;
}

template <typename _Tp>
inline const _Tp& Room<_Tp>::get_father_hull() const{
    return *m_father_hull;
}

template <typename _Tp>
inline Maze<_Tp>* Room<_Tp>::get_maze() const{
    return m_maze;
}

template <typename _Tp>
const std::vector<_Tp>& Room<_Tp>::get_vector_fields_typed_fwd() const{
    return m_vector_fields_typed_fwd;
}

template <typename _Tp>
const std::vector<_Tp>& Room<_Tp>::get_vector_fields_typed_bwd() const{
    return m_vector_fields_typed_bwd;
}

template <typename _Tp>
inline bool Room<_Tp>::is_in_deque(){
    bool result;
    omp_set_lock(&m_lock_deque);
    result = m_in_deque;
    omp_unset_lock(&m_lock_deque);
    return result;
}

template <typename _Tp>
inline bool Room<_Tp>::set_in_queue(){
    bool result = false;
    omp_set_lock(&m_lock_deque);
    if(!m_in_deque && !m_removed){
        m_in_deque = true;
        result = true;
    }
    omp_unset_lock(&m_lock_deque);
    return result;
}

template <typename _Tp>
inline void Room<_Tp>::reset_deque(){
    omp_set_lock(&m_lock_deque);
    m_in_deque = false;
    omp_unset_lock(&m_lock_deque);
}

template <typename _Tp>
inline std::vector<ibex::IntervalVector> Room<_Tp>::get_vector_fields() const{
    return m_vector_fields;
}

template <typename _Tp>
inline std::vector<ibex::IntervalMatrix> Room<_Tp>::get_vector_fields_d() const{
    return m_vector_fields_d1;
}


template <typename _Tp>
inline void Room<_Tp>::lock_contraction(){
    omp_set_lock(&m_lock_contraction);
}

template <typename _Tp>
inline void Room<_Tp>::unlock_contraction(){
    omp_unset_lock(&m_lock_contraction);
}

template <typename _Tp>
inline bool Room<_Tp>::is_removed() const{
    return m_removed;
}

template <typename _Tp>
inline size_t Room<_Tp>::get_nb_contractions() const{
    return m_nb_contract;
}

template <typename _Tp>
inline std::vector<bool> Room<_Tp>::get_vector_fields_zero(){
    return m_vector_field_zero;
}

template <typename _Tp>
inline bool Room<_Tp>::get_contain_zero_coordinate() const{
    return m_contain_zero_coordinate;
}

template <typename _Tp>
inline bool Room<_Tp>::get_contain_zero() const{
    return m_contain_zero;
}

template <typename _Tp>
inline const _Tp& Room<_Tp>::get_one_vector_fields_typed_fwd(int n_vf) const{
    return m_vector_fields_typed_fwd[n_vf];
}

template <typename _Tp>
inline const _Tp& Room<_Tp>::get_one_vector_fields_typed_bwd(int n_vf) const{
    return m_vector_fields_typed_bwd[n_vf];
}

template <typename _Tp>
inline ibex::IntervalVector Room<_Tp>::get_one_vector_fields(int n_vf) const{
    return m_vector_fields[n_vf];
}

template <typename _Tp>
inline void Room<_Tp>::set_union_hull(const _Tp &hull){
    for(Face<_Tp> *f:m_pave->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[m_maze];
        d->set_union_hull(hull);
    }
}

template <typename _Tp>
inline bool Room<_Tp>::is_first_contract() const{
    return m_first_contract;
}

template <typename _Tp>
inline bool Room<_Tp>::is_contract_vector_field() const{
    return m_contract_vector_field;
}

inline std::string print(const ibex::IntervalVector &iv){
    std::ostringstream stream;
    stream << iv;
    return stream.str();
}

inline std::string print(const ppl::C_Polyhedron &p){
    std::ostringstream stream;
    ppl::IO_Operators::operator <<(stream, p);
    return stream.str();
}

}
#include "room.tpp"

#endif // ROOM_H
