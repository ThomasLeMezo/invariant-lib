#ifndef VTK_GRAPH_H
#define VTK_GRAPH_H

#include "smartSubPaving.h"
#include "previmer3d.h"
#include "lambertgrid.h"

//#include <vtkAppendPolyData.h>
//#include <vtkCubeSource.h>
//#include <vtkXMLPolyDataWriter.h>
//#include <vtkSmartPointer.h>

class VtkMaze3D
{
public:
    /**
     * @brief Constructor of the vtk view
     * @param file_name
     * @param g
     * @param memory_optimization : will delete paves after computing rooms view
     * (use this option only if you need a unique show_maze)
     */
    VtkMaze3D(const std::string& file_name, bool memory_optimization=false);

    /**
     * @brief Export to a file the representation of a paving
     */
    void show_graph(invariant::SmartSubPaving<ibex::IntervalVector> *subpaving);

    /**
     * @brief Export to a file the representation of a maze
     * @param maze
     * @param comment
     */
    void show_maze(invariant::Maze<ibex::IntervalVector> *maze, std::string comment="");

    /**
     * @brief show_maze
     * @param file_name
     */
    void show_maze(const string &file_name);

    /**
     * @brief Export to a file the representation of selected paves
     * @param maze
     * @param position_info
     */
    void show_room_info(invariant::Maze<ibex::IntervalVector> *maze, ibex::IntervalVector position);

    /**
     * @brief serialize_maze
     * @param file_name
     */
    void serialize_maze(const string &file_name, invariant::Maze<ibex::IntervalVector> *maze);

private:
    std::string         m_file_name = "";
    bool                m_memory_optimization = false;
    int                 m_number_export = 0;
};

/**
 * @brief monteCarlos
 * @param pm3d
 * @param t0
 * @param x0
 * @param y0
**/
void monteCarlos(invariant::PreviMer3D &pm3d, double t0, double x0, double y0);

/**
 * @brief draw_map
 * @param g
 */
void draw_map(invariant::LambertGrid &g);

#endif // VTK_GRAPH_H
