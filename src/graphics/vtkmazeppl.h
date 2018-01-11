#ifndef VTKMAZEPPL_H
#define VTKMAZEPPL_H

#include "maze.h"

class VtkMazePPL
{
public:
    /**
     * @brief VtkMazePPL
     * @param file_name
     */
    VtkMazePPL(const std::string& file_name);

    /**
     * @brief show_maze
     * @param maze
     * @param comment
     */
    void show_maze(invariant::MazePPL *maze, std::string comment="");

    /**
     * @brief show_subpaving
     * @param maze
     * @param comment
     */
    void show_subpaving(invariant::MazePPL *maze, std::string comment="");

    /**
     * @brief VtkMazePPL::show_vector_field
     * @param maze
     * @param comment
     */
    void show_vector_field(invariant::MazePPL *maze, std::string comment="");

    /**
     * @brief show room info
     * @param maze
     * @param position
     */
    void show_room_info(invariant::MazePPL *maze, ibex::IntervalVector position);

private:

    std::string         m_file_name = "";
    int                 m_number_export = 0;
    int                 m_number_export_paves = 0;
    int                 m_number_export_vector_field = 0;
};

#endif // VTKMAZEPPL_H
