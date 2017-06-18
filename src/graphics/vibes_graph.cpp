#include "vibes_graph.h"
#include <ibex.h>

using namespace invariant;
using namespace ibex;
using namespace std;

Vibes_Graph::Vibes_Graph(const std::string& figure_name, Graph *g): VibesFigure(figure_name){
    if(g->dim() != 2)
        throw std::runtime_error("in [vibes_graph.cpp/Vibes_Graph()] dim of graph is not equal to 2");
    m_graph = g;
    m_overhead_factor = 0.2; // 20%
}

Vibes_Graph::Vibes_Graph(const std::string& figure_name, Graph *g, Maze* outer): Vibes_Graph(figure_name, g){
    m_maze_outer = outer;
}

Vibes_Graph::Vibes_Graph(const std::string& figure_name, Graph *g, Maze* outer, Maze* inner): Vibes_Graph(figure_name, g){
    m_maze_outer = outer;
    m_maze_inner = inner;
}

void Vibes_Graph::show() const{
    show_graph();
    if(m_maze_outer != NULL)
        show_maze_outer();

}

void Vibes_Graph::show_maze_outer() const{
    for(Pave *p:m_graph->get_paves()){
        // Draw backward
        vibes::drawBox(p->get_position(), "[blue]");

        // Draw Polygon
        vector<double> pt_x, pt_y;
        vector<tuple<int, int, bool>> oriented_path;
        oriented_path.push_back(make_tuple(0, 0, true));
        oriented_path.push_back(make_tuple(1, 1, true));
        oriented_path.push_back(make_tuple(0, 1, false));
        oriented_path.push_back(make_tuple(1, 0, false));

        for(tuple<int, int, bool> &t:oriented_path){
            Door *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_outer];
            IntervalVector d_iv = d->get_input() | d->get_output();

            if(!d_iv.is_empty()){
                if(get<2>(t)){
                    pt_x.push_back(d_iv[0].lb());
                    pt_y.push_back(d_iv[1].lb());
                    pt_x.push_back(d_iv[0].ub());
                    pt_y.push_back(d_iv[1].ub());
                }
                else{
                    pt_x.push_back(d_iv[0].ub());
                    pt_y.push_back(d_iv[1].ub());
                    pt_x.push_back(d_iv[0].lb());
                    pt_y.push_back(d_iv[1].lb());
                }
            }
        }
        vibes::drawPolygon(pt_x, pt_y, "[yellow]");

        // Draw Cone
        show_theta(p, m_maze_outer);

    }
    for(Pave *p:m_graph->get_paves_not_bisectable()){
        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
            Room *r = p->get_rooms()[m_maze_outer];
            if(r->is_empty())
                vibes::drawBox(p->get_position(), "[blue]");
        }
    }
}

void Vibes_Graph::show_theta(Pave *p, Maze* maze) const{
    IntervalVector position(p->get_position());
    double size = 0.8*min(position[0].diam(), position[1].diam())/2.0;
    Room *r = p->get_rooms()[maze];

    const vector<IntervalVector> vector_fields = r->get_vector_fields();

    for(IntervalVector r:vector_fields){
        for(Interval i:compute_theta(r[0], r[1])){
            vibes::drawSector(position[0].mid(), position[1].mid(), size, size, (-i.lb())*180.0/M_PI, (-i.ub())*180.0/M_PI, "black[gray]");
        }
    }

}

std::vector<ibex::Interval> Vibes_Graph::compute_theta(ibex::Interval dx, ibex::Interval dy) const{
    std::vector<ibex::Interval> theta_list;

    for(int i=0; i<2; i++)
        theta_list.push_back(Interval::EMPTY_SET);

    Interval theta = atan2(dy, dx);

    if(theta==(-Interval::PI|Interval::PI)){
        Interval thetaR = atan2(-dy, -dx); // PI rotation ({dx, dy} -> {-dx, -dy})
        if(thetaR.diam()<theta.diam()){
            if(thetaR.is_subset(-Interval::PI | Interval::PI)){
                theta_list[0] = (thetaR + Interval::PI) & (Interval::ZERO | Interval::PI); // theta[0] in [0, pi]
                theta_list[1] = ((thetaR + Interval::PI) & (Interval::PI | Interval::TWO_PI)) - Interval::TWO_PI; // theta[1] in [-pi, 0]
            }
            else
                throw runtime_error("in [Vibes_Graph.cpp/compute_theta] error with the computation of theta");
        }
        else
            theta_list[0] = theta;
    }
    else if(theta.is_empty())
        theta_list[0] = -Interval::PI|Interval::PI;
    else
        theta_list[0] = theta;
    if(theta_list[0].is_empty())
        throw runtime_error("in [Vibes_Graph.cpp/compute_theta] error with the computation of theta");


    if(theta_list[1].is_empty())
        theta_list.erase(theta_list.begin()+1);

    return theta_list;
}

void Vibes_Graph::show_graph() const{
    IntervalVector bounding_box(m_graph->dim(), Interval::EMPTY_SET);
    vibes::newGroup("graph_bisectable", "gray[gray]", vibesParams("figure", m_name));
    vibes::newGroup("graph_not_bisectable", "lightGray[lightGray]", vibesParams("figure", m_name));


    vibes::Params params_bisectable, params_not_bisectable;
    params_bisectable = vibesParams("figure", m_name, "group", "graph_bisectable", "FaceColor","none","EdgeColor","gray");
    params_not_bisectable = vibesParams("figure", m_name, "group", "graph_not_bisectable", "FaceColor","none","EdgeColor","lightGray");

    for(Pave*p:m_graph->get_paves()){
        ibex::IntervalVector box(p->get_position());
        vibes::drawBox(box, params_bisectable);
        bounding_box |= box;
    }
    for(Pave*p:m_graph->get_paves_not_bisectable()){
        if(!p->get_position().is_unbounded()){
            vibes::drawBox(p->get_position(), params_not_bisectable);
            bounding_box |= p->get_position();
        }
    }

    double overhead_x0 = bounding_box.diam()[0]*m_overhead_factor;
    double overhead_x1 = bounding_box.diam()[1]*m_overhead_factor;
    vibes::axisLimits(bounding_box[0].lb()-overhead_x0, bounding_box[0].ub()+overhead_x0,
            bounding_box[1].lb()-overhead_x1, bounding_box[1].ub()+overhead_x1,
            m_name);
}

namespace vibes{
inline void drawGraph(const invariant::Graph &g, Params params){
    vibes::drawPave(g.get_paves(), params);
    vibes::drawPave(g.get_paves_not_bisectable(), params);
}

inline void drawPave(const invariant::Pave &p, Params params){
    vibes::drawBox(p.get_position(), params);
}

inline void drawPave(const vector<invariant::Pave*>& l_p, Params params){
    for(Pave *p:l_p)
        vibes::drawPave(*p, params);
}

inline void drawFace(const invariant::Face &f, Params params){
    vibes::drawBox(f.get_position(), params);
}

inline void drawFace(const vector<invariant::Face *>& l_f, Params params){
    for(Face *f:l_f)
        vibes::drawFace(*f, params);
}
}
