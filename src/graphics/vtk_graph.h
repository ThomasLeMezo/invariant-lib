#ifndef VTK_GRAPH_H
#define VTK_GRAPH_H

#include "graph.h"

class Vtk_Graph
{
public:
    /**
     * @brief Constructor of the vtk view
     * @param file_name
     * @param g
     * @param memory_optimization : will delete paves after computing rooms view
     * (use this option only if you need a unique show_maze)
     */
    Vtk_Graph(const std::string& file_name, invariant::Graph *g, bool memory_optimization=false);

    /**
     * @brief Export to a file the representation of a graph
     */
    void show_graph();

    /**
     * @brief Export to a file the representation of a maze
     * @param maze
     * @param comment
     */
    void show_maze(invariant::Maze *maze, std::string comment="");

    /**
     * @brief Export to a file the representation of selected paves
     * @param maze
     * @param position_info
     */
    void show_room_info(invariant::Maze *maze, ibex::IntervalVector position);

private:
    invariant::Graph*   m_graph = NULL;

    invariant::Maze*    m_maze_outer = NULL;
    invariant::Maze*    m_maze_inner = NULL;

    std::string         m_file_name = "";
    bool                m_memory_optimization = false;
};

#endif // VTK_GRAPH_H
