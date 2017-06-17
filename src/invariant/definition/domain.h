#ifndef DOMAIN_H
#define DOMAIN_H

#include <ibex.h>
#include <utility>

#include "graph.h"
#include "maze.h"
#include "door.h"

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
     * @brief Add a box which will be removed from the domain (input & output)
     * @param box
     */
    void add_remove_box(const ibex::IntervalVector& box);

    /**
     * @brief Add a list of boxes which will be removed from the domain (input & output)
     * @param boxes
     */
    void add_remove_boxes(const std::vector<ibex::IntervalVector>& boxes);

    /**
     * @brief Add a contractor that contract boxes outside the domain (input & output)
     * @param contractor
     */
    void add_remove_contractor(ibex::Ctc *contractor);

    /**
     * @brief Add a list of contractors that contract boxes outside the domain (input & output)
     * @param contractors
     */
    void add_remove_contractors(const std::vector<ibex::Ctc*>& contractors);

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
     * @brief Add a box which will be removed from the domain (input)
     * @param box
     */
    void add_remove_box_input(const ibex::IntervalVector& box);

    /**
     * @brief Add a list of boxes which will be removed from the domain (input)
     * @param boxes
     */
    void add_remove_boxes_input(const std::vector<ibex::IntervalVector>& boxes);

    /**
     * @brief Add a contractor that contract boxes outside the domain (input)
     * @param contractor
     */
    void add_remove_contractor_input(ibex::Ctc *contractor);

    /**
     * @brief Add a list of contractors that contract boxes outside the domain (input)
     * @param contractors
     */
    void add_remove_contractors_input(const std::vector<ibex::Ctc*>& contractors);

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
     * @brief Add a box which will be removed from the domain (output)
     * @param box
     */
    void add_remove_box_output(const ibex::IntervalVector& box);

    /**
     * @brief Add a list of boxes which will be removed from the domain (output)
     * @param boxes
     */
    void add_remove_boxes_output(const std::vector<ibex::IntervalVector>& boxes);

    /**
     * @brief Add a contractor that contract boxes outside the domain (output)
     * @param contractor
     */
    void add_remove_contractor_output(ibex::Ctc *contractor);

    /**
     * @brief Add a list of contractors that contract boxes outside the domain (output)
     * @param contractors
     */
    void add_remove_contractors_output(const std::vector<ibex::Ctc*>& contractors);

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
     * @brief Contract a door according to the domain definition
     * @param door
     */
    void contract(Door &door);

private:
    /**
     * @brief Contract an IntervalVector according to the list of removed conditions
     * @param iv
     * @param remove_boxes
     * @param remove_mazes
     * @param remove_contractor
     */
    void contract(ibex::IntervalVector &iv,
                  Maze *maze,
                  const std::vector<ibex::IntervalVector> &remove_boxes,
                  const std::vector<std::pair<Maze*, bool>> &remove_mazes,
                  const std::vector<ibex::Ctc*> &remove_contractor);

private:
    Graph * m_graph;

    std::vector<ibex::IntervalVector> m_remove_boxes_input, m_remove_boxes_output;
    std::vector<std::pair<invariant::Maze*, bool>> m_remove_mazes_input, m_remove_mazes_output;
    std::vector<ibex::Ctc*> m_remove_contractors_input, m_remove_contractors_output;
};
}

namespace invariant{
inline void Domain::add_remove_box_input(const ibex::IntervalVector& box){
    m_remove_boxes_input.push_back(box);
}
inline void Domain::add_remove_box_output(const ibex::IntervalVector& box){
    m_remove_boxes_output.push_back(box);
}
inline void Domain::add_remove_box(const ibex::IntervalVector& box){
    add_remove_box_input(box);
    add_remove_box_output(box);
}

inline void Domain::add_remove_boxes_input(const std::vector<ibex::IntervalVector>& boxes){
    m_remove_boxes_input.insert(m_remove_boxes_input.end(), boxes.begin(), boxes.end());
}
inline void Domain::add_remove_boxes_output(const std::vector<ibex::IntervalVector>& boxes){
    m_remove_boxes_output.insert(m_remove_boxes_output.end(), boxes.begin(), boxes.end());
}
inline void Domain::add_remove_boxes(const std::vector<ibex::IntervalVector>& boxes){
    add_remove_boxes_input(boxes);
    add_remove_boxes_output(boxes);
}

inline void Domain::add_remove_contractor_input(ibex::Ctc *contractor){
    m_remove_contractors_input.push_back(contractor);
}
inline void Domain::add_remove_contractor_output(ibex::Ctc *contractor){
    m_remove_contractors_output.push_back(contractor);
}
inline void Domain::add_remove_contractor(ibex::Ctc *contractor){
    add_remove_contractor_input(contractor);
    add_remove_contractor_output(contractor);
}

inline void Domain::add_remove_contractors_input(const std::vector<ibex::Ctc *> &contractors){
    m_remove_contractors_input.insert(m_remove_contractors_input.end(), contractors.begin(), contractors.end());
}
inline void Domain::add_remove_contractors_output(const std::vector<ibex::Ctc *> &contractors){
    m_remove_contractors_output.insert(m_remove_contractors_output.end(), contractors.begin(), contractors.end());
}
inline void Domain::add_remove_contractors(const std::vector<ibex::Ctc *> &contractors){
    add_remove_contractors_output(contractors);
    add_remove_contractors_input(contractors);
}

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
