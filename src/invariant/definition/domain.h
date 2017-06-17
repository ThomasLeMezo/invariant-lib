#ifndef DOMAIN_H
#define DOMAIN_H

#include <ibex.h>
#include <utility>

#include "graph.h"
#include "maze.h"
#include "door.h"
#include "pave_node.h"

namespace invariant{
class Graph; // declared only for friendship
class Maze; // declared only for friendship
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
     * @brief Add a maze where non empty (or empty depending of complementary parameter) doors (input | output)
     *        will be removed from the domain (input & output)
     * @param maze
     * @param bool complementary : true = remove empty, false = remove non empty
     */
    void add_remove_maze(invariant::Maze *maze, bool complementary);

    /**
     * @brief Add a list of pair <maze, bool> where non empty (or empty depending of complementary parameter) doors (input | output)
     *        will be removed from the domain (input & output)
     * @param pairs (Maze, bool), see add_remove_maze(invariant::Maze *maze, bool complementary) function
     */
    void add_remove_mazes(const std::vector<std::pair<invariant::Maze*, bool>>& pairs);

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

    // *************** Contractors *********************

    /**
     * @brief Contract all the doors of the maze according to the separator contractor
     * @param maze
     * @param pave_node
     * @param l
     */
    void contract_separator(Maze *maze, Pave_node *pave_node, bool all_out, std::vector<Pave*> &l);


private:
    Graph * m_graph;

    std::vector<std::pair<invariant::Maze*, bool>> m_remove_mazes_input, m_remove_mazes_output;
    std::vector<ibex::Sep *> m_sep_input, m_sep_output;
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

}

#endif // DOMAIN_H
