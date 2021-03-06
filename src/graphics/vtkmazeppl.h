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
    void show_maze(invariant::MazePPL *maze, std::string comment="", bool complementary=false);

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

    /**
     * @brief simu_trajectory
     * @param f
     * @param position_init
     * @param t_max
     * @param dt
     */
//    void simu_trajectory(ibex::Function *f, const std::vector<double>& position_init, const double &t_max, const double &dt, const std::vector<double>& visu_scale=std::vector<double>{1.0, 1.0, 1.0});

    void simu_trajectory(invariant::Dynamics &d, const ibex::IntervalVector &space, const size_t &nb_traj, const double &t_max, const double &dt, const std::vector<double> &visu_scale=std::vector<double>{1.0, 1.0, 1.0});

private:

    std::string         m_file_name = "";
    int                 m_number_export = 0;
    int                 m_number_export_paves = 0;
    int                 m_number_export_vector_field = 0;
    size_t              m_traj_num = 0;
};

#endif // VTKMAZEPPL_H
