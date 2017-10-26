#ifndef Vibes_Graph_H
#define Vibes_Graph_H

#include <ibex/ibex_IntervalVector.h>
#include "smartSubPaving.h"
#include "maze.h"
#include "pave.h"
#include "VibesFigure.h"
#include "vibes/vibes.h"

class Vibes_Graph: public VibesFigure
{

public:
    enum VIBES_GRAPH_TYPE{VIBES_GRAPH_INNER, VIBES_GRAPH_OUTER, VIBES_GRAPH_OUTER_AND_INNER};
public:
    Vibes_Graph(const std::string& figure_name, invariant::SmartSubPaving *g);
    Vibes_Graph(const std::string& figure_name, invariant::Maze* maze, VIBES_GRAPH_TYPE type=VIBES_GRAPH_OUTER);
    Vibes_Graph(const std::string& figure_name, invariant::Maze* outer, invariant::Maze* inner);
    ~Vibes_Graph(){}

    virtual void show() const;

    void get_room_info(invariant::Maze *maze, const ibex::IntervalVector& position) const;
    void get_room_info(invariant::Maze *maze, double x, double y) const;

    void show_room_info(invariant::Maze *maze, const ibex::IntervalVector &position) const;

    ///
    /// \brief drawCircle for easier integration with the python binding
    /// \param x_center
    /// \param y_center
    /// \param radius
    /// \param params
    ///
    void drawCircle(double x_center, double y_center, double radius, std::string params) const;

private:
    void show_graph() const;
    void show_maze_outer() const;
    void show_maze_inner() const;
    void show_theta(invariant::Pave *p, invariant::Maze *maze) const;

    void show_maze_outer_inner() const;

    void draw_room_outer(invariant::Pave *p) const;
    void draw_room_inner(invariant::Pave *p) const;

    std::vector<ibex::Interval> compute_theta(ibex::Interval dx, ibex::Interval dy) const;
private:
    invariant::SmartSubPaving*   m_subpaving = NULL;
    double              m_overhead_factor;

    invariant::Maze*    m_maze_outer = NULL;
    invariant::Maze*    m_maze_inner = NULL;

    std::vector<std::tuple<int, int, bool>> m_oriented_path;

    VIBES_GRAPH_TYPE    m_type = VIBES_GRAPH_OUTER;

};

namespace vibes{
    VIBES_FUNC_COLOR_PARAM_1(drawGraph,const invariant::SmartSubPaving &, g)
    VIBES_FUNC_COLOR_PARAM_1(drawPave,const invariant::Pave &, p)
    VIBES_FUNC_COLOR_PARAM_1(drawFace,const invariant::Face &, f)
    VIBES_FUNC_COLOR_PARAM_1(drawPave,const std::vector<invariant::Pave*> &, l_p)
    VIBES_FUNC_COLOR_PARAM_1(drawFace,const std::vector<invariant::Face*> &, l_f)
}

#endif // Vibes_Graph_H
