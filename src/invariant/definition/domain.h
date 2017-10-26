#ifndef DOMAIN_H
#define DOMAIN_H

#include <ibex/ibex_IntervalVector.h>
#include <ibex/ibex_Sep.h>
#include <utility>

#include "smartSubPaving.h"
#include "maze.h"
#include "room.h"
#include "pave_node.h"

namespace invariant{

enum DOMAIN_SEP{SEP_INSIDE, SEP_OUTSIDE, SEP_UNKNOWN};
enum DOMAIN_PROPAGATION_START{LINK_TO_INITIAL_CONDITION, NOT_LINK_TO_INITIAL_CONDITION};

enum DOMAIN_INITIALIZATION{FULL_DOOR, FULL_WALL};

class SmartSubPaving; // declared only for friendship
class Maze; // declared only for friendship
class Pave_node; // declared only for friendship
class Room; // declared only for friendship
class Door; // declared only for friendship
class Domain
{
public:
    /**
     * @brief Constructor of a domain
     * @param paving
     * @param link : specify if the initial condition in the case of a propagation maze is always link to the yellow zone
     */
    Domain(SmartSubPaving* paving, DOMAIN_INITIALIZATION domain_init, DOMAIN_PROPAGATION_START link=NOT_LINK_TO_INITIAL_CONDITION);

    // *************** Intput & Output *********************

    /**
     * @brief Add a maze where non empty (or empty depending of complementary parameter) doors (input and output)
     *        will be removed from the domain (input & output)
     * @param maze
     * @param bool complementary : true = remove empty, false = remove non empty
     */
    void add_remove_maze(invariant::Maze *maze, bool complementary);

    /**
     * @brief Add a list of pair <maze, bool> where non empty (or empty depending of complementary parameter) doors (input and output)
     *        will be removed from the domain (input & output)
     * @param pairs (Maze, bool), see add_remove_maze(invariant::Maze *maze, bool complementary) function
     */
    void add_remove_mazes(const std::vector<std::pair<invariant::Maze*, bool>>& pairs);

    /**
     * @brief Set the separator to contract the Doors of a maze (output and input)
     * @param sep
     */
    void set_sep(ibex::Sep* sep);

    // *************** Intput *********************

    /**
     * @brief Add a maze where non empty (or empty depending of complementary parameter) doors (input | output)
     *        will be removed from the domain (input)
     * @param maze
     * @param bool complementary : true = remove empty, false = remove non empty
     */
    void add_remove_maze_input(invariant::Maze *maze, bool complementary);

    /**
     * @brief Add a list of pair <maze, bool> where non empty (or empty depending of complementary parameter) doors (input | output)
     *        will be removed from the domain (input)
     * @param pairs (Maze, bool), see add_remove_maze(invariant::Maze *maze, bool complementary) function
     */
    void add_remove_mazes_input(const std::vector<std::pair<invariant::Maze*, bool>>& pairs);

    /**
     * @brief Set the separator to contract the Doors of a maze (outside and inside)
     * @param sep
     */
    void set_sep_input(ibex::Sep* sep);

    // *************** Output *********************

    /**
     * @brief Add a maze where non empty (or empty depending of complementary parameter) doors (input | output)
     *        will be removed from the domain (output)
     * @param maze
     * @param bool complementary : true = remove empty, false = remove non empty
     */
    void add_remove_maze_output(invariant::Maze *maze, bool complementary);

    /**
     * @brief Add a list of pair <maze, bool> where non empty (or empty depending of complementary parameter) doors (input | output)
     *        will be removed from the domain (output)
     * @param pairs (Maze, bool), see add_remove_maze(invariant::Maze *maze, bool complementary) function
     */
    void add_remove_mazes_output(const std::vector<std::pair<invariant::Maze*, bool>>& pairs);

    /**
     * @brief Set the separator to contract the Doors of a maze (output)
     * @param sep
     */
    void set_sep_output(ibex::Sep* sep);

    // *************** Contractors *********************

    /**
     * @brief Contract all the doors of the maze according to the separator contractor
     * @param maze
     * @param pave_node
     * @param l
     */
    void contract_domain(Maze *maze, std::vector<Room *> &list_room_deque);

    /**
     * @brief Return the paving associated with this domain
     * @return
     */
    SmartSubPaving* get_subpaving() const;

    /**
     * @brief Set "in" to true if there are incoming paths on the border
     * @param in
     */
    void set_border_path_in(bool in);

    /**
     * @brief Set "out" to true if there are outcoming paths on the border
     * @param out
     */
    void set_border_path_out(bool out);

    /**
     * @brief add maze for domain intersection
     * @param maze
     */
    void add_maze_inter(Maze *maze);

    /**
     * @brief add maze list for domain intersection
     * @param maze_list
     */
//    void add_maze_inter(std::vector<Maze*> maze_list);

    /**
     * @brief add maze for domain union
     * @param maze
     */
    void add_maze_union(Maze *maze);

    /**
     * @brief add maze list for domain union
     * @param maze_list
     */
//    void add_maze_union(std::vector<Maze*> maze_list);

    /**
     * @brief Get the initialization condition on Room (Full door or Full wall)
     * @return
     */
    DOMAIN_INITIALIZATION get_init() const;

private:
    /**
     * @brief Contract all output or input doors of the maze according to the separator contractor
     * @param maze
     * @param pave_node
     * @param l
     * @param output : true => contract output, false => contract input
     */
    void contract_separator(Maze *maze, Pave_node *pave_node, std::vector<Room *> &list_room_deque, bool output, DOMAIN_SEP accelerator);

    /**
     * @brief Contract the boarders according to the options
     * @param maze
     */
    void contract_border(Maze *maze, std::vector<Room*> &list_room_deque);

    /**
     * @brief Contract the domain by intersecting with maze list
     * @param maze
     */
    void contract_inter_maze(Maze *maze);

    /**
     * @brief Contract the domain by union with maze list
     * @param maze
     */
    void contract_union_maze(Maze *maze);

private:
    SmartSubPaving * m_subpaving;

    ibex::Sep* m_sep_input = NULL;
    ibex::Sep* m_sep_output = NULL;

    bool m_border_path_in = true;
    bool m_border_path_out = true;

    DOMAIN_PROPAGATION_START m_link_start = NOT_LINK_TO_INITIAL_CONDITION;
    DOMAIN_INITIALIZATION m_domain_init = FULL_DOOR;

    std::vector<Maze *> m_maze_list_inter;
    std::vector<Maze *> m_maze_list_union;
};
}

namespace invariant{

inline DOMAIN_INITIALIZATION Domain::get_init() const{
    return m_domain_init;
}

inline void Domain::set_sep(ibex::Sep* sep){
    m_sep_input = sep;
    m_sep_output = sep;
}

inline void Domain::set_sep_input(ibex::Sep* sep){
    m_sep_input = sep;
}

inline void Domain::set_sep_output(ibex::Sep* sep){
    m_sep_output = sep;
}

inline SmartSubPaving* Domain::get_subpaving() const{
    return m_subpaving;
}

inline void Domain::set_border_path_in(bool in){
    m_border_path_in = in;
}

inline void Domain::set_border_path_out(bool out){
    m_border_path_out = out;
}

inline void Domain::add_maze_inter(Maze *maze){
    m_maze_list_inter.push_back(maze);
}

inline void Domain::add_maze_union(Maze *maze){
    m_maze_list_union.push_back(maze);
}

//inline void Domain::add_maze_inter(std::vector<Maze *> maze_list){
//    m_maze_list_inter.insert(m_maze_list_inter.end(), maze_list.begin(), maze_list.end());
//}

//inline void Domain::add_maze_union(std::vector<Maze *> maze_list){
//    m_maze_list_union.insert(m_maze_list_union.end(), maze_list.begin(), maze_list.end());
//}

}

#endif // DOMAIN_H
