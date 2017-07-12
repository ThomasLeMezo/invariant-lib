#include "vibes_graph.h"
#include <ibex.h>
#include "ostream"

using namespace invariant;
using namespace ibex;
using namespace std;

Vibes_Graph::Vibes_Graph(const std::string& figure_name, Graph *g): VibesFigure(figure_name){
    if(g->dim() != 2)
        throw std::runtime_error("in [vibes_graph.cpp/Vibes_Graph()] dim of graph is not equal to 2");
    m_graph = g;
    m_overhead_factor = 0.2; // 20%

    m_oriented_path.push_back(std::make_tuple(0, 0, true));
    m_oriented_path.push_back(std::make_tuple(1, 1, true));
    m_oriented_path.push_back(std::make_tuple(0, 1, false));
    m_oriented_path.push_back(std::make_tuple(1, 0, false));
}

Vibes_Graph::Vibes_Graph(const std::string& figure_name, Graph *g, Maze* outer, VIBES_GRAPH_TYPE type): Vibes_Graph(figure_name, g){
    if(type == VIBES_GRAPH_OUTER)
        m_maze_outer = outer;
    else
        m_maze_inner = outer;
}

Vibes_Graph::Vibes_Graph(const std::string& figure_name, Graph *g, Maze* outer, Maze* inner): Vibes_Graph(figure_name, g){
    m_maze_outer = outer;
    m_maze_inner = inner;
}

void Vibes_Graph::show() const{
    show_graph();
    if(m_maze_outer != NULL && m_maze_inner == NULL)
        show_maze_outer();
    if(m_maze_outer == NULL && m_maze_inner != NULL)
        show_maze_inner();
    else if(m_maze_outer != NULL && m_maze_inner != NULL)
        show_maze_outer_inner();

}

void Vibes_Graph::draw_room_inner(Pave *p) const{
    // Draw Polygon
    vector<double> pt_x, pt_y;

    for(const tuple<int, int, bool> &t:m_oriented_path){
        Door *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_inner];
        IntervalVector d_iv = d->get_input() | d->get_output();
        IntervalVector* d_iv_list;
        int nb_vec = d->get_face()->get_position().diff(d_iv, d_iv_list);

        if(nb_vec == 1){
            IntervalVector d_iv_c(d_iv_list[0]);
            if(!d_iv_c.is_empty()){
                if(get<2>(t)){
                    pt_x.push_back(d_iv_c[0].lb());
                    pt_y.push_back(d_iv_c[1].lb());
                    pt_x.push_back(d_iv_c[0].ub());
                    pt_y.push_back(d_iv_c[1].ub());
                }
                else{
                    pt_x.push_back(d_iv_c[0].ub());
                    pt_y.push_back(d_iv_c[1].ub());
                    pt_x.push_back(d_iv_c[0].lb());
                    pt_y.push_back(d_iv_c[1].lb());
                }
            }
        }
    }
    vibes::drawPolygon(pt_x, pt_y, "[#FF00FF]");

    // Draw Cone
    show_theta(p, m_maze_inner);

}

void Vibes_Graph::draw_room_outer(Pave *p) const{
    // Draw backward
    vibes::drawBox(p->get_position(), "[blue]");

    // Draw Polygon
    vector<double> pt_x, pt_y;

    for(const tuple<int, int, bool> &t:m_oriented_path){
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

void Vibes_Graph::show_maze_outer() const{
    for(Pave *p:m_graph->get_paves()){
        draw_room_outer(p);
    }
    for(Pave *p:m_graph->get_paves_not_bisectable()){
        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
            Room *r = p->get_rooms()[m_maze_outer];
            //            if(!r->is_removed()){
            draw_room_outer(p);
            //            }
            if(r->is_empty())
                vibes::drawBox(p->get_position(), "[blue]");
        }
    }
}

void Vibes_Graph::show_maze_inner() const{
    for(Pave *p:m_graph->get_paves()){
        draw_room_inner(p);
    }
    for(Pave *p:m_graph->get_paves_not_bisectable()){
        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
            Room *r = p->get_rooms()[m_maze_inner];
            //            if(!r->is_removed()){
            draw_room_inner(p);
            //            }
            if(r->is_empty())
                vibes::drawBox(p->get_position(), "[#FF00FF]");
        }
    }
}

void Vibes_Graph::show_maze_outer_inner() const{
    for(Pave *p:m_graph->get_paves()){
        draw_room_outer(p);

        Room *r_inner = p->get_rooms()[m_maze_inner];
        if(!r_inner->is_full()){
            draw_room_inner(p);
        }
    }

    for(Pave *p:m_graph->get_paves_not_bisectable()){
        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
            Room *r_outer = p->get_rooms()[m_maze_outer];
            Room *r_inner = p->get_rooms()[m_maze_inner];
            if(r_outer->is_empty())
                vibes::drawBox(p->get_position(), "[blue]");
            if(r_inner->is_empty())
                vibes::drawBox(p->get_position(), "[#FF00FF]");
        }
    }
}

void Vibes_Graph::show_theta(Pave *p, Maze* maze) const{
    IntervalVector position(p->get_position());
    double size = 0.8*min(position[0].diam(), position[1].diam())/2.0;
    Room *r = p->get_rooms()[maze];

    const vector<IntervalVector> vector_fields = r->get_vector_fields();
    IntervalVector empty(2, Interval::EMPTY_SET);

    if(vector_fields.size()>0){
        for(IntervalVector r:vector_fields){
            if(r == empty)
                vibes::drawBox(position[0].mid()+Interval(-size, +size), position[1].mid()+Interval(-size, +size), "black[gray]");
            else{
                for(Interval i:compute_theta(r[0], r[1])){
                    vibes::drawSector(position[0].mid(), position[1].mid(), size, size, (-i.lb())*180.0/M_PI, (-i.ub())*180.0/M_PI, "black[gray]");
                }
            }
        }
    }
    else{
        vibes::drawSector(position[0].mid(), position[1].mid(), size, size, -180, 180, "black[gray]");
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

void Vibes_Graph::get_room_info(invariant::Maze *maze, const ibex::IntervalVector& position) const{
    std::vector<invariant::Pave*> pave_list;
    m_graph->get_room_info(maze, position, pave_list);
    for(invariant::Pave* p:pave_list){
        vibes::drawCircle(p->get_position()[0].mid(), p->get_position()[1].mid(),
                0.6*min(p->get_position()[0].diam()/2.0, p->get_position()[1].diam()/2.0), "[green]");
    }
}

void Vibes_Graph::get_room_info(invariant::Maze *maze, double x, double y) const{
    IntervalVector pos(2);
    pos[0] = Interval(x);
    pos[1] = Interval(y);
    get_room_info(maze, pos);
}

void Vibes_Graph::show_room_info(invariant::Maze *maze, const IntervalVector& position) const{
    std::vector<invariant::Pave*> pave_list;
    m_graph->get_room_info(maze, position, pave_list);
    for(invariant::Pave* p:pave_list){
        vibes::drawCircle(p->get_position()[0].mid(), p->get_position()[1].mid(),
                0.6*min(p->get_position()[0].diam()/2.0, p->get_position()[1].diam()/2.0), "[green]", vibesParams("figure", m_name));
    }

    for(invariant::Pave* p:pave_list){
        IntervalVector p_position = p->get_position();
        double p_diam[2] = {p_position[0].diam(), p_position[1].diam()};
        double offset[2] = {p_diam[0]*0.05, p_diam[1]*0.05};

        // New Figure
        ostringstream name;
        name << p_position;
        vibes::newFigure(name.str());
        vibes::setFigureProperties(vibesParams("width", 512,"height", 512));

        // Draw Pave
        draw_room_outer(p);

        // Draw Doors
        for(int face=0; face<2; face++){
            for(int sens=0; sens<2; sens++){
                Face *f=p->get_faces()[face][sens];
                Door *d = f->get_doors()[maze];

                IntervalVector input = d->get_input();
                IntervalVector output = d->get_output();

                input[face] += 1.0*Interval((sens==1)?(offset[face]):(-offset[face])) + Interval(-offset[face]/4.0, offset[face]/4.0);
                output[face] += 2.0*Interval((sens==1)?(offset[face]):(-offset[face])) + Interval(-offset[face]/4.0, offset[face]/4.0);

                vibes::drawBox(input, "red[red]");
                vibes::drawBox(output, "blue[blue]");
            }
        }
        vibes::axisLimits(p_position[0].lb()-3*offset[0], p_position[0].ub()+3*offset[0],
                p_position[1].lb()-3*offset[1], p_position[1].ub()+3*offset[1]);
    }
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
