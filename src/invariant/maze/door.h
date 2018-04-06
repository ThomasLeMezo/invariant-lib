#ifndef DOOR_H
#define DOOR_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include <omp.h>

#include <iostream>
#include <sstream>      // std::ostringstream
#include <map>

#include "domain.h"
#include "face.h"
#include "room.h"
#include "maze.h"
#include "pave.h"


namespace ppl=Parma_Polyhedra_Library;

namespace invariant {

template <typename _Tp> class Door;
using DoorPPL = Door<Parma_Polyhedra_Library::C_Polyhedron>;
using DoorIBEX = Door<ibex::IntervalVector>;

template <typename _Tp> class Face;
template <typename _Tp> class Room;
template <typename _Tp> class Maze;
template <typename _Tp> class Pave;
template <typename _Tp> class Domain;

template <typename _Tp=ibex::IntervalVector>
class Door
{
public:
    /**
     * @brief Constructor of a Door
     * @param face
     * By default doors are set open
     */
    Door(invariant::Face<_Tp>* face, invariant::Room<_Tp> *room);

    /**
     * @brief Destructor of a Door
     */
    ~Door();

    /**
     * @brief reset
     */
    void reset();

    /**
     * @brief Get the public read input door
     * @return iv
     */
    const _Tp get_input() const;

    /**
     * @brief Get the public read output door
     * @return iv
     */
    const _Tp get_output() const;

    /**
     * @brief Set the input door
     * @param iv_input
     */
    void set_input_private(const _Tp& iv_input);

    /**
     * @brief Set the output door
     * @param iv_output
     */
    void set_output_private(const _Tp& iv_output);

    /**
     * @brief set union hull
     * @param hull
     */
    void set_union_hull(const _Tp& hull);

    /**
     * @brief Get the input door
     * @return iv
     */
    const _Tp& get_input_private() const;

    /**
     * @brief Get the output door
     * @return iv
     */
    const _Tp& get_output_private() const;

    /**
     * @brief Synchronize public and private
     */
    void synchronize();

    /**
     * @brief Getter to the associated Face
     * @return
     */
    Face<_Tp> * get_face() const;

    /**
     * @brief Getter to the associated Room
     * @return
     */
    Room<_Tp> *get_room() const;

    /**
     * @brief Set the private output doors to empty
     */
    void set_empty_private_output();

    /**
     * @brief Set the private output doors to full
     */
    void set_full_private_output();

    /**
     * @brief Set the input private door to empty
     */
    void set_empty_private_input();

    /**
     * @brief Set the input private door to full
     */
    void set_full_private_input();

    /**
     * @brief Set all input and output private doors to empty
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
     * @brief Set full all input & output private doors according to
     * vector field possibility
     */
    void set_full_possible_private();

    /**
     * @brief Contract its private door according to neighbors
     */
    bool contract_continuity_private();

    /**
     * @brief If there is a difference between private & public doors
     * add the neighbor room to the list
     * @param list_rooms
     */
    bool analyze_change(std::vector<Room<_Tp> *>&list_rooms);

    /**
     * @brief Return true if input & output doors are empty
     * @return
     */
    bool is_empty() const;
    bool is_empty_output() const;
    bool is_empty_input() const;

    bool is_empty_private() const;

    /**
     * @brief Return true if input & output doors are full (ie equal to position)
     * @return
     */
    bool is_full() const;
    bool is_full_output() const;
    bool is_full_input() const;

    bool is_full_private() const;
    bool is_full_private_output() const;
    bool is_full_private_input() const;

    /**
     * @brief Return true if the union of input & output doors are full
     * @return
     */
    bool is_full_union() const;

    /**
     * @brief Set if this door is a possible out for propagation
     * @param val
     */
    void push_back_possible_out(bool val);

    /**
     * @brief Set if the vector field is collinear to the face
     * @param val
     */
    void push_back_collinear_vector_field(bool val);

    /**
     * @brief Set if the vector field union is collinear to the face
     * @param val
     */
    void set_collinear_vector_field_union(bool val);

    /**
     * @brief Get if this door is a possible out for propagation
     * @return
     */
    const std::vector<bool> &is_possible_out() const;

    /**
     * @brief Set if this door is a possible in for propagation
     * @param val
     */
    void push_back_possible_in(bool val);

    /**
     * @brief Get if this door is a possible in for propagation
     * @return
     */
    const std::vector<bool>& is_possible_in() const;

    /**
     * @brief Get if the vector field is collinear to the door
     * @return
     */
    const std::vector<bool>& is_collinear() const;

    /**
     * @brief Get if the vector field union is collinear to the door
     * @return
     */
    const bool &is_collinear_union() const;

    /**
     * @brief Set the dimensions where the n-ith vector field cross zero
     * @param zeros
     */
    void push_back_zeros_in_vector_field(std::vector<bool> zeros);

    /**
     * @brief Set the dimensions where the n-ith vector field cross zero
     * @param zeros
     */
    void push_back_zeros_in_vector_field_union(std::vector<bool> zeros);

    /**
     * @brief Return the list of dimensions where the i-th vector field cross zero
     * @param vector_field_id
     * @return
     */
    const std::vector<bool> &get_where_zeros(size_t vector_field_id) const;

    /**
     * @brief Return the list of dimensions where the i-th vector field cross zero
     * @param vector_field_id
     * @return
     */
    const std::vector<bool> &get_where_zeros_union() const;

    /**
     * @brief Reduce de size of the Door if inactive (delete private IV)
     */
    void set_removed();

    /**
     * @brief Get the hull of the door
     * @return
     */
    const _Tp get_hull() const;

    /**
     * @brief operator &=
     * @param d1
     * @param d2
     * @return
     */
    friend Door& operator&=(Door& d1, const Door& d2){
        d1.set_input_private(d1.get_input_private() & d2.get_input());
        d1.set_output_private(d1.get_output_private() & d2.get_output());
        return d1;
    }

    /**
     * @brief operator |=
     * @param d1
     * @param d2
     * @return
     */
    friend Door& operator|=(Door& d1, const Door& d2){
        d1.set_input_private(d1.get_input_private() | d2.get_input());
        d1.set_output_private(d1.get_output_private() | d2.get_output());
        return d1;
    }

protected:
    _Tp m_input_public;
    _Tp m_output_public; //input and output doors public
    _Tp *m_input_private;
    _Tp *m_output_private; //input and output doors private (for contraction)
    Face<_Tp> *               m_face = nullptr; // pointer to the associated face
    Room<_Tp> *               m_room = nullptr; // pointer to the associated face
    mutable omp_lock_t   m_lock_read;

    std::vector<bool>    m_possible_out;
    std::vector<bool>    m_possible_in;
    bool                 m_possible_in_union = false; // For at least one command
    bool                 m_possible_out_union = false;
    std::vector<bool>    m_collinear_vector_field;
    bool                 m_collinear_vector_field_union;

    // Idea : add a father hull in each door ?
};
}

namespace invariant{

ppl::C_Polyhedron iv_2_polyhedron(const ibex::IntervalVector& iv);
ibex::IntervalVector polyhedron_2_iv(const ppl::C_Polyhedron& p);

//template <typename _Tp>
//inline std::ostream& operator<<(std::ostream& stream, const invariant::Door<_Tp>& d){
//    return stream;
//}

template <typename _Tp>
inline const _Tp Door<_Tp>::get_input() const{
    omp_set_lock(&m_lock_read);
    _Tp tmp(m_input_public);
    omp_unset_lock(&m_lock_read);
    return tmp;
}

template <typename _Tp>
inline const _Tp Door<_Tp>::get_output() const{
    omp_set_lock(&m_lock_read);
    _Tp tmp(m_output_public);
    omp_unset_lock(&m_lock_read);
    return tmp;
}

template <typename _Tp>
inline const _Tp& Door<_Tp>::get_input_private() const{
    return *m_input_private;
}

template <typename _Tp>
inline const _Tp& Door<_Tp>::get_output_private() const{
    return *m_output_private;
}

template <typename _Tp>
inline Face<_Tp> * Door<_Tp>::get_face() const{
    return m_face;
}

template <typename _Tp>
inline Room<_Tp> * Door<_Tp>::get_room() const{
    return m_room;
}

template <typename _Tp>
inline void Door<_Tp>::push_back_possible_in(bool val){
    m_possible_in.push_back(val);
    m_possible_in_union |= val;
}

template <typename _Tp>
inline void Door<_Tp>::push_back_possible_out(bool val){
    m_possible_out.push_back(val);
    m_possible_out_union |= val;
}

template <typename _Tp>
inline void Door<_Tp>::push_back_collinear_vector_field(bool val){
    m_collinear_vector_field.push_back(val);
}

template <typename _Tp>
inline void Door<_Tp>::set_collinear_vector_field_union(bool val){
    m_collinear_vector_field_union = val;
}

template <typename _Tp>
inline const std::vector<bool>& Door<_Tp>::is_collinear() const{
    return m_collinear_vector_field;
}

template <typename _Tp>
inline const bool& Door<_Tp>::is_collinear_union() const{
    return m_collinear_vector_field_union;
}

template <typename _Tp>
inline const std::vector<bool>& Door<_Tp>::is_possible_out() const{
    return m_possible_out;
}

template <typename _Tp>
inline const std::vector<bool>& Door<_Tp>::is_possible_in() const{
    return m_possible_in;
}

template <typename _Tp>
inline bool Door<_Tp>::is_empty_private() const{
    if(m_input_private->is_empty() && m_output_private->is_empty())
        return true;
    else
        return false;
}

template <typename _Tp>
inline bool Door<_Tp>::is_empty() const{
    bool result;
    omp_set_lock(&m_lock_read);
    if(m_input_public.is_empty() && m_output_public.is_empty())
        result = true;
    else
        result = false;
    omp_unset_lock(&m_lock_read);
    return result;
}

template <typename _Tp>
inline bool Door<_Tp>::is_empty_output() const{
    bool result;
    omp_set_lock(&m_lock_read);
    if(m_output_public.is_empty())
        result = true;
    else
        result = false;
    omp_unset_lock(&m_lock_read);
    return result;
}

template <typename _Tp>
inline bool Door<_Tp>::is_empty_input() const{
    bool result;
    omp_set_lock(&m_lock_read);
    if(m_input_public.is_empty())
        result = true;
    else
        result = false;
    omp_unset_lock(&m_lock_read);
    return result;
}

template <typename _Tp>
inline void Door<_Tp>::set_empty_private(){
    set_empty_private_input();
    set_empty_private_output();
}

template <typename _Tp>
inline void Door<_Tp>::set_union_hull(const _Tp& hull){
    *m_input_private |= hull & m_face->get_position_typed();
    *m_output_private |= hull & m_face->get_position_typed();
}



/// ******************  Other functions ****************** ///

inline std::ostream& operator<<(std::ostream& stream, const invariant::DoorIBEX& d){
    std::ostringstream input, output;
    input << d.get_input();
    output << d.get_output();
    stream << std::left << "input = " << std::setw(46) << input.str() << " output = " << std::setw(46) << output.str();
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const invariant::DoorPPL& d){
    stream << "input = ";
    ppl::IO_Operators::operator <<(stream, d.get_input());
    stream << " output = ";
    ppl::IO_Operators::operator <<(stream, d.get_output());
    return stream;
}

inline bool is_subset(const ibex::IntervalVector &iv1, const ibex::IntervalVector &iv2){
    return iv1.is_subset(iv2);
}

inline bool is_subset(const ppl::C_Polyhedron &p1, const ppl::C_Polyhedron &p2){
    return p2.contains(p1);
}

inline void union_widening(ppl::C_Polyhedron* p1, const ppl::C_Polyhedron& p2){
    ppl::C_Polyhedron p_result(p2);
    p_result.BHRZ03_widening_assign(*p1);
    *p1 = p_result;
}

inline void union_widening(ibex::IntervalVector* iv1, const ibex::IntervalVector& iv2){
    *(iv1) |= iv2;
}

}

#include "door.tpp"

#endif // DOOR_H

