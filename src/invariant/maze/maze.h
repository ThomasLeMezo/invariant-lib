#ifndef MAZE_H
#define MAZE_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>
#include "diff_expbox.h"

#include <deque>
#include <iostream>

#include <omp.h>

#include "dynamics.h"
#include "domain.h"
#include "room.h"
#include "smartSubPaving.h"
#include "pave.h"
#include "booleantree.h"

namespace invariant {

template <typename _TpR, typename _TpF, typename _TpD> class Maze;
using MazePPL = Maze<Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron>;
using MazeIBEX = Maze<ibex::IntervalVector,ibex::IntervalVector,ibex::IntervalVector>;
using MazeEXP = Maze<ibex::IntervalVector,ExpVF,ExpPoly>;

class Dynamics; // declared only for friendship
template <typename _TpR, typename _TpF, typename _TpD> class Room;
template <typename _TpR, typename _TpF, typename _TpD> class Domain;
template <typename _TpR, typename _TpF, typename _TpD> class SmartSubPaving;
template <typename _TpR, typename _TpF, typename _TpD> class Pave;
template <typename _TpR, typename _TpF, typename _TpD> class Face;
template <typename _TpR, typename _TpF, typename _TpD> class Door;

template <typename _TpR=ibex::IntervalVector, typename _TpF=ibex::IntervalVector, typename _TpD=ibex::IntervalVector>
class Maze
{
public:
    /**
     * @brief Constructor of a Maze
     * @param g
     * @param f_vect
     * @param type of operation (forward, backward or both)
     */
    Maze(invariant::Domain<_TpR,_TpF,_TpD> *domain, Dynamics *dynamics);

    /**
     * @brief Maze destructor
     */
    ~Maze();

    /**
     * @brief Getter to the Domain
     * @return
     */
    Domain<_TpR,_TpF,_TpD> * get_domain() const;

    /**
     * @brief Getter to the SmartSubPaving
     * @return
     */
    SmartSubPaving<_TpR,_TpF,_TpD> * get_subpaving() const;

    /**
     * @brief Getter to the dynamics
     * @return
     */
    Dynamics *get_dynamics() const;

    /**
     * @brief Contract the Maze
     * @return the number of contractions
     */
    int contract(size_t nb_operations=0);

    /**
     * @brief Contract the Maze by intersecting with other domain mazes
     */
//    void contract_inter(Maze *maze_n);

    /**
     * @brief Add a Room to the deque BUT DO NOT CHECK if already in
     * (to short locking the deque)
     * @param r
     */
    void add_to_deque(Room<_TpR,_TpF,_TpD> *r);

    /**
     * @brief Add a list of rooms to the deque (check if already in)
     * @param list_rooms
     */
    void add_rooms(const std::vector<Room<_TpR,_TpF,_TpD> *> &list_rooms);

    /**
     * @brief Return true if some trajectory can escape from the search space
     * @return
     */
    bool is_escape_trajectories();

    /**
     * @brief Return true if all rooms of the maze are empty
     * @return
     */
    bool is_empty();

    /**
     * @brief is_enable_father_hull
     * @return
     */
    bool is_enable_father_hull() const;

    /**
     * @brief get the bounding box
     * @return
     */
    ibex::IntervalVector get_bounding_box();

    /**
     * @brief compute_vector_field
     */
    void compute_vector_field();

    /**
     * @brief get widening limit
     * @return
     */
    size_t get_widening_limit() const;

    /**
     * @brief get contraction limit
     * @return
     */
    size_t get_contraction_limit() const;

    /**
     * @brief get limit contraction door
     * @return
     */
    bool get_limit_contraction_door() const;

    /**
     * @brief set widening limit
     * @param limit
     */
    void set_widening_limit(size_t limit);

    /**
     * @brief set contraction limit
     * @param limit
     */
    void set_contraction_limit(size_t limit);

    /**
     * @brief set a limit to door contraction (equivalent widening)
     * @param val
     */
    void set_enable_contraction_limit(bool val);

    /**
     * @brief Set nb_operation to 0
     */
    void reset_nb_operations();

    /**
     * @brief Set enable contract domain variable
     * @param val
     */
    void set_enable_contract_domain(bool val);

    /**
     * @brief set_enable_father_hull
     * @param val
     */
    void set_enable_father_hull(bool val);

    /**
     * @brief add_initial_room
     * @param room_pt
     */
    void add_initial_room(Room<_TpR,_TpF,_TpD>* room_pt);

    /**
     * @brief reset_initial_room_list
     */
    void reset_initial_room_list();

    /**
     * @brief get_initial_room_list
     * @return
     */
    std::vector<Room<_TpR,_TpF,_TpD>*> get_initial_room_list();

    /**
     * @brief add_hybrid_room
     * @param room_pt
     */
    void add_hybrid_room(Room<_TpR,_TpF,_TpD>* r);

    /**
     * @brief reset_hybrid_room_list
     */
    void reset_hybrid_room_list();

    /**
     * @brief get_hybrid_room_list
     * @return
     */
    std::vector<Room<_TpR,_TpF,_TpD>*> get_hybrid_room_list();

    /**
     * @brief get_contract_once
     * @return
     */
    bool get_contract_once() const;

    /**
     * @brief set_contract_vector_field
     * @param val
     * @return
     */
    void set_enable_contract_vector_field(bool val);

    /**
     * @brief get_contract_vector_field
     * @return
     */
    bool get_contract_vector_field() const;

    /**
     * @brief get_boolean_tree_removing
     * @return
     */
    BooleanTree<_TpR,_TpF,_TpD> * get_boolean_tree_removing() const;

    /**
     * @brief get_contraction_step
     * @return
     */
    int get_contraction_step() const;

    /**
     * @brief set_boolean_tree_removing
     */
    void set_boolean_tree_removing(BooleanTree<_TpR,_TpF,_TpD> *);

    /**
     * @brief discover_hybrid_rooms
     */
    void discover_hybrid_rooms();

private:
    invariant::Domain<_TpR,_TpF,_TpD> *    m_domain = nullptr;
    SmartSubPaving<_TpR,_TpF,_TpD>  *    m_subpaving = nullptr; // SmartSubPaving associated with this maze
    Dynamics *  m_dynamics = nullptr;

    std::deque<Room<_TpR,_TpF,_TpD> *> m_deque_rooms;
    omp_lock_t  m_deque_access;
    omp_lock_t  m_initial_rooms_access;
    omp_lock_t  m_hybrid_rooms_access;

    bool    m_espace_trajectories = true;
    bool    m_contract_once = false;
    bool m_empty = false;
    int m_contraction_step = 0;
    bool m_enable_father_hull = false;

    size_t m_nb_operations=0;
    size_t m_widening_limit=1000;
    size_t m_contraction_limit=1000;
    bool m_enable_contract_domain = true;
    bool m_limit_contraction_door = false;

    bool m_contract_vector_field = false;

    BooleanTree<_TpR,_TpF,_TpD> *m_boolean_tree_removing = nullptr;

    std::vector<Room<_TpR,_TpF,_TpD>*> m_initial_rooms_list;

    std::vector<Room<_TpR,_TpF,_TpD>*> m_hybrid_rooms_list;
};
}

namespace invariant{

template <typename _TpR, typename _TpF, typename _TpD>
inline BooleanTree<_TpR,_TpF,_TpD> * Maze<_TpR,_TpF,_TpD>::get_boolean_tree_removing() const{
    return m_boolean_tree_removing;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline int Maze<_TpR,_TpF,_TpD>::get_contraction_step() const{
    return m_contraction_step;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::set_boolean_tree_removing(BooleanTree<_TpR,_TpF,_TpD> * tree){
    m_boolean_tree_removing = tree;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::set_widening_limit(size_t limit){
    m_widening_limit = limit;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::set_contraction_limit(size_t limit){
    m_contraction_limit = limit;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::set_enable_contraction_limit(bool val){
    m_limit_contraction_door = val;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::set_enable_contract_domain(bool val){
    m_enable_contract_domain = val;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::set_enable_father_hull(bool val){
    m_enable_father_hull = val;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline size_t Maze<_TpR,_TpF,_TpD>::get_widening_limit() const{
    return m_widening_limit;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline size_t Maze<_TpR,_TpF,_TpD>::get_contraction_limit() const{
    return m_contraction_limit;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline bool Maze<_TpR,_TpF,_TpD>::get_limit_contraction_door() const{
    return m_limit_contraction_door;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline Domain<_TpR,_TpF,_TpD> * Maze<_TpR,_TpF,_TpD>::get_domain() const{
    return m_domain;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline Dynamics * Maze<_TpR,_TpF,_TpD>::get_dynamics() const{
    return m_dynamics;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline SmartSubPaving<_TpR,_TpF,_TpD> * Maze<_TpR,_TpF,_TpD>::get_subpaving() const{
    return m_subpaving;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::add_to_deque(Room<_TpR,_TpF,_TpD> *r){
    omp_set_lock(&m_deque_access);
    m_deque_rooms.push_back(r);
    omp_unset_lock(&m_deque_access);
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::add_initial_room(Room<_TpR,_TpF,_TpD>* room_pt){
    omp_set_lock(&m_initial_rooms_access);
    m_initial_rooms_list.push_back(room_pt);
    omp_unset_lock(&m_initial_rooms_access);
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::reset_initial_room_list(){   
    m_initial_rooms_list.clear();
}

template <typename _TpR, typename _TpF, typename _TpD>
inline std::vector<Room<_TpR,_TpF,_TpD>*> Maze<_TpR,_TpF,_TpD>::get_initial_room_list(){
    return m_initial_rooms_list;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::add_hybrid_room(Room<_TpR,_TpF,_TpD>* r){
    omp_set_lock(&m_hybrid_rooms_access);
    m_hybrid_rooms_list.push_back(r);
    omp_unset_lock(&m_hybrid_rooms_access);
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::reset_hybrid_room_list(){
    for(Room<_TpR,_TpF,_TpD>*r:m_hybrid_rooms_list)
        r->reset_hybrid_list();
    m_hybrid_rooms_list.clear();
}

template <typename _TpR, typename _TpF, typename _TpD>
inline std::vector<Room<_TpR,_TpF,_TpD>*> Maze<_TpR,_TpF,_TpD>::get_hybrid_room_list(){
    return m_hybrid_rooms_list;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline bool Maze<_TpR,_TpF,_TpD>::get_contract_once() const{
    return m_contract_once;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline bool Maze<_TpR,_TpF,_TpD>::get_contract_vector_field() const{
    return m_contract_vector_field;
}

template <typename _TpR, typename _TpF, typename _TpD>
inline void Maze<_TpR,_TpF,_TpD>::set_enable_contract_vector_field(bool val){
    m_contract_vector_field = val;
}

template <typename _TpD>
Parma_Polyhedra_Library::Thread_Init* initialize_thread();

template <typename _TpD>
void delete_thread_init(Parma_Polyhedra_Library::Thread_Init* thread_init);

}

#include "maze.tpp"

#endif // MAZE_H
