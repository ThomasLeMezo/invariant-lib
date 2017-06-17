#ifndef DOOR_H
#define DOOR_H

#include <ibex.h>
#include "face.h"
#include "room.h"
#include <omp.h>

namespace invariant {
class Face; // declared only for friendship
class Room; // declared only for friendship
class Door
{
public:
    /**
     * @brief Constructor of a Door
     * @param face
     */
    Door(Face * face, Room *romm);

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
     * @brief Set all the output doors to empty
     */
    void set_empty_private_output();

    /**
     * @brief Set all the input doors to empty
     */
    void set_empty_private_input();

    /**
     * @brief Set all input and output doors to empty
     */
    void set_empty_private();

    /**
     * @brief Contract its private door according to neighbors
     */
    void contract_continuity_private();

    /**
     * @brief If there is a difference between private & public doors
     * add the neighbor room to the list
     * @param list_rooms
     */
    void analyze_change(std::vector<Room *>&list_rooms);


private:
    ibex::IntervalVector m_input_public, m_output_public; //input and output doors public
    ibex::IntervalVector m_input_private, m_output_private; //input and output doors private (for contraction)
    Face *               m_face = NULL; // pointer to the associated face
    Room *               m_room = NULL; // pointer to the associated face
    mutable omp_lock_t   m_lock_read;
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
    return m_input_private;
}

inline const ibex::IntervalVector& Door::get_output_private() const{
    return m_output_private;
}

inline void Door::set_input_private(const ibex::IntervalVector& iv_input){
    m_input_private = iv_input;
}

inline void Door::set_output_private(const ibex::IntervalVector& iv_output){
    m_output_private = iv_output;
}

inline Face * Door::get_face() const{
    return m_face;
}

inline Room * Door::get_room() const{
    return m_room;
}

inline void Door::set_empty_private_output(){
    m_output_private.set_empty();
}

inline void Door::set_empty_private_input(){
    m_input_private.set_empty();
}

inline void Door::set_empty_private(){
    m_output_private.set_empty();
    m_input_private.set_empty();
}
}
#endif // DOOR_H
