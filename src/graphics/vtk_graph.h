#ifndef VTK_GRAPH_H
#define VTK_GRAPH_H

#include "graph.h"

class Vtk_Graph
{
public:
    /**
     * @brief Constructor of the vtk view
     */
    Vtk_Graph(const std::string& file_name, invariant::Graph *g);

    void show_graph();

    void show_maze(invariant::Maze *maze, std::string comment="");

private:
    invariant::Graph*   m_graph = NULL;

    invariant::Maze*    m_maze_outer = NULL;
    invariant::Maze*    m_maze_inner = NULL;

    std::string         m_file_name = "";
};

#endif // VTK_GRAPH_H
