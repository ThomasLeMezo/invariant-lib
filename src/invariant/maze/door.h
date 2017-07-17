#ifndef DOOR_H
#define DOOR_H

#include <ibex.h>
#include "face.h"
#include "room.h"
#include "maze.h"
#include <omp.h>
#include <iostream>
#include <iomanip>

namespace invariant {
class Face; // declared only for friendship
class Room; // declared only for friendship
class Maze; // declared only for friendship
class Door
{
public:
    /**
     * @brief Constructor of a Door
     * @param face
     */
    Door(Face * face, Room *room);

    /**
     * @brief Destructor of a Door
     */
    ~Door();

    /**
     * @brief Get the public read input door
     * @return iv
     */
    const ibex::IntervalVector get_input() const;

    /**
     * @brief Get the public read output door
     * @return iv
     */
    const ibex::IntervalVector get_output() const;

    /**
     * @brief Set the input door
     * @param iv_input
     */
    void set_input_private(const ibex::IntervalVector& iv_input);

    /**
     * @brief Set the output door
     * @param iv_output
     */
    void set_output_private(const ibex::IntervalVector& iv_output);

    /**
     * @brief Get the input door
     * @return iv
     */
    const ibex::IntervalVector& get_input_private() const;

    /**
     * @brief Get the output door
     * @return iv
     */
    const ibex::IntervalVector& get_output_private() const;

    /**
     * @brief Synchronize public and private
     */
    void synchronize();

    /**
     * @brief Getter to the associated Face
     * @return
     */
    Face * get_face() const;

    /**
     * @brief Getter to the associated Room
     * @return
     */
    Room *get_room() const;

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
    void analyze_change(std::vector<Room *>&list_rooms);

    /**
     * @brief Return true if input & output doors are empty
     * @return
     */
    bool is_empty();

    /**
     * @brief Return true if input & output doors are full (ie equal to position)
     * @return
     */
    bool is_full();

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
    const std::vector<bool> &is_possible_in() const;

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

protected:
    ibex::IntervalVector m_input_public;
    ibex::IntervalVector m_output_public; //input and output doors public
    ibex::IntervalVector *m_input_private;
    ibex::IntervalVector *m_output_private; //input and output doors private (for contraction)
    Face *               m_face = NULL; // pointer to the associated face
    Room *               m_room = NULL; // pointer to the associated face
    mutable omp_lock_t   m_lock_read;

    std::vector<bool>    m_possible_out;
    std::vector<bool>    m_possible_in;
    std::vector<bool>    m_collinear_vector_field;
    std::vector<std::vector<bool>> m_zeros_in_vector_fields;
};
}

namespace invariant{

inline const ibex::IntervalVector Door::get_input() const{
    omp_set_lock(&m_lock_read);
    ibex::IntervalVector tmp(m_input_public);
    omp_unset_lock(&m_lock_read);
    return tmp;
}

inline const ibex::IntervalVector Door::get_output() const{
    omp_set_lock(&m_lock_read);
    ibex::IntervalVector tmp(m_output_public);
    omp_unset_lock(&m_lock_read);
    return tmp;
}

inline const ibex::IntervalVector& Door::get_input_private() const{
    return *m_input_private;
}

inline const ibex::IntervalVector& Door::get_output_private() const{
    return *m_output_private;
}

inline void Door::set_input_private(const ibex::IntervalVector& iv_input){
    *m_input_private = iv_input;
}

inline void Door::set_output_private(const ibex::IntervalVector& iv_output){
    *m_output_private = iv_output;
}

inline Face * Door::get_face() const{
    return m_face;
}

inline Room * Door::get_room() const{
    return m_room;
}

inline void Door::set_empty_private_output(){
    m_output_private->set_empty();
}

inline void Door::set_empty_private_input(){
    m_input_private->set_empty();
}

inline void Door::set_empty_private(){
    m_output_private->set_empty();
    m_input_private->set_empty();
}

inline bool Door::is_empty(){
    if(m_input_public.is_empty() && m_output_public.is_empty())
        return true;
    else
        return false;
}

inline std::ostream& operator<< (std::ostream& stream, const Door& d){
    std::ostringstream input, output;
    input << d.get_input();
    output << d.get_output();
    stream << std::left << "input = " << std::setw(46) << input.str() << " output = " << std::setw(46) << output.str();
    return stream;
}

inline void Door::push_back_possible_in(bool val){
    m_possible_in.push_back(val);
}

inline void Door::push_back_possible_out(bool val){
    m_possible_out.push_back(val);
}

inline void Door::push_back_collinear_vector_field(bool val){
    m_collinear_vector_field.push_back(val);
}

inline void Door::push_back_zeros_in_vector_field(std::vector<bool> zeros){
    m_zeros_in_vector_fields.push_back(zeros);
}

inline const std::vector<bool>& Door::get_where_zeros(size_t vector_field_id) const{
    return m_zeros_in_vector_fields[vector_field_id];
}

inline const std::vector<bool>& Door::is_collinear() const{
    return m_collinear_vector_field;
}

inline const std::vector<bool>& Door::is_possible_out() const{
    return m_possible_out;
}

inline const std::vector<bool>& Door::is_possible_in() const{
    return m_possible_in;
}
}
#endif // DOOR_H
