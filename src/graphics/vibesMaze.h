#ifndef VibesMaze_H
#define VibesMaze_H

#include <ibex_IntervalVector.h>
#include <ppl.hh>

#include "smartSubPaving.h"
#include "maze.h"
#include "pave.h"
#include "domain.h"
#include "face.h"
#include "door.h"
#include "eulerianmaze.h"

#include "VibesFigure.h"
#include "vibes/vibes.h"

class VibesMaze: public VibesFigure
{

public:
    enum VIBES_MAZE_TYPE{VIBES_MAZE_INNER, VIBES_MAZE_OUTER, VIBES_MAZE_OUTER_AND_INNER, VIBES_MAZE_EULERIAN};
public:
    VibesMaze(const std::string& figure_name, invariant::SmartSubPavingIBEX *g);
    VibesMaze(const std::string& figure_name, invariant::MazeIBEX* maze, VIBES_MAZE_TYPE type=VIBES_MAZE_OUTER);
    VibesMaze(const std::string& figure_name, invariant::MazeIBEX* outer, invariant::MazeIBEX* inner, bool both_wall=false);
    VibesMaze(const std::string& figure_name, std::vector<invariant::MazeIBEX*> &outer, std::vector<invariant::MazeIBEX*> &inner, bool both_wall=false);
    VibesMaze(const std::string& figure_name, invariant::EulerianMazeIBEX* e);
    ~VibesMaze(){}

    virtual void show() const;

    void get_room_info(invariant::MazeIBEX *maze, const ibex::IntervalVector& position) const;
    void get_room_info(invariant::MazeIBEX *maze, double x, double y) const;

    void show_room_info(invariant::MazeIBEX *maze, const ibex::IntervalVector &position);

    /**
     * @brief drawCircle for easier integration with the python binding
     * @param x_center
     * @param y_center
     * @param radius
     * @param params
     */
    void drawCircle(double x_center, double y_center, double radius, std::string params) const;

    /**
     * @brief drawBox for easier integration with the python binding
     * @param x_min
     * @param x_max
     * @param y_min
     * @param y_max
     * @param params
     */
    void drawBox(double x_min, double x_max, double y_min, double y_max, std::string params) const;

    /**
     * @brief drawBox for easier integration with the python binding
     * @param box
     * @param params
     */
    void drawBox(const ibex::IntervalVector &box, std::string params) const;

    /**
     * @brief set_enable_cone
     * @param val
     */
    void set_enable_cone(bool val);

    /**
     * @brief get_volume
     * @return
     */
    double get_volume(const bool inner=false) const;

    /**
     * @brief get_volume_pave
     * @param p
     * @param inner
     * @return
     */
    double get_volume_pave(invariant::PaveIBEX *p, const bool inner=false) const;

    /**
     * @brief add_stat
     * @param step
     * @param time
     * @param volume_outer
     * @param volume_inner
     */
    void add_stat(size_t step, double time, double volume_outer, double volume_inner);

    /**
     * @brief save_stat_to_file
     * @param namefile
     */
    void save_stat_to_file(std::string namefile);

    /**
     * @brief set_scale
     * @param scale_x
     * @param scale_y
     */
    void set_scale(double scale_x, double scale_y);

    /**
     * @brief set_offset
     * @param offset_x
     * @param offset_y
     */
    void set_offset(double offset_x, double offset_y);

private:
    void show_graph() const;
    void show_maze_outer() const;
    void show_maze_inner() const;
    void show_theta(invariant::PaveIBEX *p, invariant::MazeIBEX *maze) const;

    void show_maze_outer_inner() const;

    void show_eulerian_maze() const;

    void draw_room_outer(invariant::PaveIBEX *p) const;
    void draw_room_inner(invariant::PaveIBEX *p) const;
    void draw_room_inner_outer(invariant::PaveIBEX *p) const;

    std::vector<ibex::Interval> compute_theta(ibex::Interval dx, ibex::Interval dy) const;
private:
    invariant::SmartSubPavingIBEX*   m_subpaving = nullptr;
    double              m_overhead_factor;


    std::vector<invariant::MazeIBEX*> m_maze_outer;
    std::vector<invariant::MazeIBEX*> m_maze_inner;
//    invariant::MazeIBEX*    m_maze_outer = nullptr;
//    invariant::MazeIBEX*    m_maze_inner = nullptr;
    invariant::EulerianMazeIBEX* m_eulerian_maze = nullptr;

    std::vector<std::tuple<int, int, bool>> m_oriented_path;

    VIBES_MAZE_TYPE    m_type = VIBES_MAZE_OUTER;

    bool m_both_wall = false;
    bool m_enable_cones = true;

    ibex::IntervalVector m_scale_factor;
    ibex::IntervalVector m_offset;

    // Stat
    std::vector<double> m_memory_step, m_memory_time, m_memory_volume_outer, m_memory_volume_inner;

};

namespace vibes{
    VIBES_FUNC_COLOR_PARAM_1(drawGraph,const invariant::SmartSubPavingIBEX &, g)
    VIBES_FUNC_COLOR_PARAM_1(drawPave,const invariant::PaveIBEX &, p)
    VIBES_FUNC_COLOR_PARAM_1(drawFace,const invariant::FaceIBEX &, f)
    VIBES_FUNC_COLOR_PARAM_1(drawPave,const std::vector<invariant::PaveIBEX*> &, l_p)
    VIBES_FUNC_COLOR_PARAM_1(drawFace,const std::vector<invariant::FaceIBEX*> &, l_f)
}

inline void VibesMaze::set_enable_cone(bool val){
    m_enable_cones = val;
}

inline void VibesMaze::set_scale(double scale_x, double scale_y){
    m_scale_factor[0] = ibex::Interval(scale_x);
    m_scale_factor[1] = ibex::Interval(scale_y);
}

inline void VibesMaze::set_offset(double offset_x, double offset_y){
    m_offset[0] = ibex::Interval(offset_x);
    m_offset[1] = ibex::Interval(offset_y);
}

#endif // VibesMaze_H
