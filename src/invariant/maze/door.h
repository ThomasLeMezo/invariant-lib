#ifndef DOOR_H
#define DOOR_H

#include <ibex/ibex_IntervalVector.h>

#include <ppl.hh>

#include <omp.h>
#include <iostream>
#include <sstream>      // std::ostringstream
#include <iomanip>

#include "../definition/domain.h"
#include "../smartSubPaving/face.h"
#include "room.h"
#include "maze.h"
#include "../smartSubPaving/pave.h"

#define IBEX_PPL_PRECISION 1e6

namespace ppl=Parma_Polyhedra_Library;

namespace invariant {

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

    /**
     * @brief Return true if input & output doors are full (ie equal to position)
     * @return
     */
    bool is_full() const;

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
     * @brief Set the dimensions where the n-ith vector field cross zero
     * @param zeros
     */
    void push_back_zeros_in_vector_field(std::vector<bool> zeros);

    /**
     * @brief Return the list of dimensions where the i-th vector field cross zero
     * @param vector_field_id
     * @return
     */
    const std::vector<bool> &get_where_zeros(size_t vector_field_id) const;

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
    Face<_Tp> *               m_face = NULL; // pointer to the associated face
    Room<_Tp> *               m_room = NULL; // pointer to the associated face
    mutable omp_lock_t   m_lock_read;

    std::vector<bool>    m_possible_out;
    std::vector<bool>    m_possible_in;
    std::vector<bool>    m_collinear_vector_field;
    std::vector<std::vector<bool>> m_zeros_in_vector_fields;
};
}

namespace invariant{

ppl::C_Polyhedron iv_2_polyhedron(const ibex::IntervalVector& iv);

template <typename _Tp>
inline std::ostream& operator<<(std::ostream& stream, const invariant::Door<_Tp>& d){
    return stream;
}

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
inline void Door<_Tp>::set_input_private(const _Tp& iv_input){
    *m_input_private = iv_input;
}

template <typename _Tp>
inline void Door<_Tp>::set_output_private(const _Tp& iv_output){
    *m_output_private = iv_output;
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
}

template <typename _Tp>
inline void Door<_Tp>::push_back_possible_out(bool val){
    m_possible_out.push_back(val);
}

template <typename _Tp>
inline void Door<_Tp>::push_back_collinear_vector_field(bool val){
    m_collinear_vector_field.push_back(val);
}

template <typename _Tp>
inline void Door<_Tp>::push_back_zeros_in_vector_field(std::vector<bool> zeros){
    m_zeros_in_vector_fields.push_back(zeros);
}

template <typename _Tp>
inline const std::vector<bool>& Door<_Tp>::get_where_zeros(size_t vector_field_id) const{
    return m_zeros_in_vector_fields[vector_field_id];
}

template <typename _Tp>
inline const std::vector<bool>& Door<_Tp>::is_collinear() const{
    return m_collinear_vector_field;
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
inline bool Door<_Tp>::is_empty() const{
    if(m_input_public.is_empty() && m_output_public.is_empty())
        return true;
    else
        return false;
}

template <typename _Tp>
inline void Door<_Tp>::set_empty_private(){
    set_empty_private_input();
    set_empty_private_output();
}

}

/// ******************  Other functions ****************** ///
// out of invariant namespace
inline std::ostream& operator<<(std::ostream& stream, const invariant::Door<ibex::IntervalVector>& d){
    std::ostringstream input, output;
    input << d.get_input();
    output << d.get_output();
    stream << std::left << "input = " << std::setw(46) << input.str() << " output = " << std::setw(46) << output.str();
    return stream;
}

ppl::C_Polyhedron& operator&=(ppl::C_Polyhedron& p1, const ppl::C_Polyhedron& p2);

ppl::C_Polyhedron& operator|=(ppl::C_Polyhedron& p1, const ppl::C_Polyhedron& p2);

#include "door.tpp"

#endif // DOOR_H
