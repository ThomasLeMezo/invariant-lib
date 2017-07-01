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

    void show_graph();

    void show_maze(invariant::Maze *maze, std::string comment="");

private:
    invariant::Graph*   m_graph = NULL;

    invariant::Maze*    m_maze_outer = NULL;
    invariant::Maze*    m_maze_inner = NULL;

    std::string         m_file_name = "";
    bool                m_memory_optimization = false;
};

#endif // VTK_GRAPH_H
