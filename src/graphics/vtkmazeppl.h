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

private:

    std::string         m_file_name = "";
    int                 m_number_export = 0;
};

#endif // VTKMAZEPPL_H
