#ifndef DOMAIN_H
#define DOMAIN_H

#include <ibex.h>
#include <utility>

#include "graph.h"
#include "maze.h"
#include "room.h"
#include "pave_node.h"

namespace invariant{
class Graph; // declared only for friendship
class Maze; // declared only for friendship
class Pave_node; // declared only for friendship
class Room; // declared only for friendship
class Door; // declared only for friendship
class Domain
{
public:
    /**
     * @brief Constructor of a domain
     * @param maze
     */
    Domain(Graph* graph);

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
    void contract_separator(Maze *maze, std::vector<Room *> &list_room_not_empty);

    Graph* get_graph() const;

private:
    /**
     * @brief Contract all output or input doors of the maze according to the separator contractor
     * @param maze
     * @param pave_node
     * @param l
     * @param output : true => contract output, false => contract input
     */
    void contract_separator(Maze *maze, Pave_node *pave_node, bool all_out, std::vector<Room *> &list_pave_not_empty, bool output);

private:
    Graph * m_graph;

    std::vector<std::pair<invariant::Maze*, bool>> m_remove_mazes_input, m_remove_mazes_output;
    ibex::Sep* m_sep_input = NULL;
    ibex::Sep* m_sep_output = NULL;
};
}

namespace invariant{
inline void Domain::add_remove_maze_input(Maze *maze, bool complementary){
    m_remove_mazes_input.push_back(std::make_pair(maze, complementary));
}

inline void Domain::add_remove_maze_output(Maze *maze, bool complementary){
    m_remove_mazes_output.push_back(std::make_pair(maze, complementary));
}

inline void Domain::add_remove_maze(Maze *maze, bool complementary){
    add_remove_maze_output(maze, complementary);
    add_remove_maze_input(maze, complementary);
}

inline void Domain::add_remove_mazes_input(const std::vector<std::pair<Maze *, bool> > &pairs){
    m_remove_mazes_input.insert(m_remove_mazes_input.begin(), pairs.begin(), pairs.end());
}

inline void Domain::add_remove_mazes_output(const std::vector<std::pair<Maze *, bool> > &pairs){
    m_remove_mazes_output.insert(m_remove_mazes_output.begin(), pairs.begin(), pairs.end());
}

inline void Domain::add_remove_mazes(const std::vector<std::pair<Maze *, bool> > &pairs){
    add_remove_mazes_input(pairs);
    add_remove_mazes_output(pairs);
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

inline Graph* Domain::get_graph() const{
    return m_graph;
}

}

#endif // DOMAIN_H
