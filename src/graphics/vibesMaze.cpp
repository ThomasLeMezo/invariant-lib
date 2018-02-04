#include "vibesMaze.h"

#include <ostream>

using namespace invariant;
using namespace ibex;
using namespace std;

VibesMaze::VibesMaze(const std::string& figure_name, invariant::SmartSubPavingIBEX *g): VibesFigure(figure_name){
    if(g->dim() != 2)
        throw std::runtime_error("in [vibes_graph.cpp/VibesMaze()] dim of paving is not equal to 2");
    m_subpaving = g;
    m_overhead_factor = 0.0; // 20%

    m_oriented_path.push_back(std::make_tuple(0, 0, true));
    m_oriented_path.push_back(std::make_tuple(1, 1, true));
    m_oriented_path.push_back(std::make_tuple(0, 1, false));
    m_oriented_path.push_back(std::make_tuple(1, 0, false));
}

VibesMaze::VibesMaze(const std::string& figure_name, invariant::MazeIBEX* maze, VIBES_MAZE_TYPE type): VibesMaze(figure_name, maze->get_subpaving()){
    if(type == VIBES_MAZE_OUTER){
        m_maze_outer = maze;
        m_type = VIBES_MAZE_OUTER;
    }
    else{
        m_maze_inner = maze;
        m_type = VIBES_MAZE_INNER;
    }
}

VibesMaze::VibesMaze(const std::string& figure_name, invariant::MazeIBEX* outer, invariant::MazeIBEX* inner): VibesMaze(figure_name, outer->get_subpaving()){
    m_maze_outer = outer;
    m_maze_inner = inner;
    m_type = VIBES_MAZE_OUTER_AND_INNER;
}

VibesMaze::VibesMaze(const std::string& figure_name, invariant::EulerianMazeIBEX *e): VibesMaze(figure_name, e->get_paving()){
    m_eulerian_maze = e;
    m_type = VIBES_MAZE_EULERIAN;
}

void VibesMaze::show() const{
    show_graph();
    if(m_maze_outer != nullptr && m_maze_inner == nullptr)
        show_maze_outer();
    if(m_maze_outer == nullptr && m_maze_inner != nullptr)
        show_maze_inner();
    else if(m_maze_outer != nullptr && m_maze_inner != nullptr)
        show_maze_outer_inner();
    else if(m_eulerian_maze != nullptr);
        show_eulerian_maze();

}

void VibesMaze::draw_room_inner(PaveIBEX *p) const{
    // Draw backward
    //    if(m_type == VIBES_MAZE_INNER) // Otherwise draw box only when outer
    //        vibes::drawBox(p->get_position(), "black[]");

    // Draw Polygon
    vector<double> pt_x, pt_y;

    for(const tuple<int, int, bool> &t:m_oriented_path){
        invariant::DoorIBEX *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_inner];
        IntervalVector d_iv(d->get_input() | d->get_output());
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
        delete[] d_iv_list;
    }
    if(!pt_x.empty())
        vibes::drawPolygon(pt_x, pt_y, "black[#FF00FF]");

    pt_x.clear(); pt_y.clear();
    for(const tuple<int, int, bool> &t:m_oriented_path){
        DoorIBEX *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_inner];
        IntervalVector d_iv(d->get_input() | d->get_output());

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
    if(!pt_x.empty())
        vibes::drawPolygon(pt_x, pt_y, "black[yellow]");

    // Draw Cone
    show_theta(p, m_maze_inner);
}

void VibesMaze::draw_room_outer(PaveIBEX *p) const{
    // Draw backward
    vibes::drawBox(p->get_position(), "black[blue]");

    // Draw Polygon
    vector<double> pt_x, pt_y;

    invariant::RoomIBEX *r = p->get_rooms()[m_maze_outer];
    if(!r->is_removed() && r->get_contain_zero()){
        vibes::drawBox(p->get_position(), "black[yellow]");
    }
    else{
        for(const tuple<int, int, bool> &t:m_oriented_path){
            DoorIBEX *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_outer];
            IntervalVector d_iv(d->get_input() | d->get_output());

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
        if(pt_x.size()>2)
            vibes::drawPolygon(pt_x, pt_y, "black[yellow]");

        //        if(r->is_initial_door_input()){
        //            vibes::drawBox(r->get_initial_door_input(), "black[red]");
        //        }
        //        if(r->is_initial_door_output()){
        //            vibes::drawBox(r->get_initial_door_output(), "black[red]");
        //        }
    }

    // Draw Cone
    show_theta(p, m_maze_outer);
}

void VibesMaze::draw_room_inner_outer(PaveIBEX *p) const{
    // Draw backward
    //    if(m_type == VIBES_MAZE_INNER) // Otherwise draw box only when outer
    //        vibes::drawBox(p->get_position(), "black[]");

    /// **************** DRAW OUTER **************** ///
    vector<double> pt_x, pt_y;
    for(const tuple<int, int, bool> &t:m_oriented_path){
        IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
        if(m_type != VIBES_MAZE_EULERIAN){
            DoorIBEX *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_outer];
            d_iv = d->get_input() | d->get_output();
        }
        else{
            for(size_t i=0; i<m_eulerian_maze->get_maze_outer_fwd().size(); i++){
                DoorIBEX *d= p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_eulerian_maze->get_maze_outer_fwd()[i]];
                IntervalVector d_tmp = d->get_input() | d->get_output();
                d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_eulerian_maze->get_maze_outer_bwd()[m_eulerian_maze->get_maze_outer_bwd().size()-1-i]];
                d_tmp &= d->get_input() | d->get_output();
                d_iv |= d_tmp;
            }
        }

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
    if(!pt_x.empty())
        vibes::drawPolygon(pt_x, pt_y, "black[yellow]");

    /// **************** DRAW INNER **************** ///
    pt_x.clear(); pt_y.clear();
    for(const tuple<int, int, bool> &t:m_oriented_path){
        IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
        if(m_type != VIBES_MAZE_EULERIAN){
            DoorIBEX *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_inner];
            d_iv = d->get_input() | d->get_output();
        }
        else{
            d_iv = IntervalVector(2);
            for(size_t i=0; i<m_eulerian_maze->get_maze_inner_fwd().size(); i++){
                DoorIBEX *d= p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_eulerian_maze->get_maze_inner_fwd()[i]];
                IntervalVector d_tmp = d->get_input() | d->get_output();
                d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_eulerian_maze->get_maze_inner_bwd()[m_eulerian_maze->get_maze_inner_bwd().size()-1-i]];
                d_tmp |= d->get_input() | d->get_output();
                d_iv &= d_tmp;
            }
        }


        IntervalVector* d_iv_list;
        int nb_vec = p->get_faces()[get<0>(t)][get<1>(t)]->get_position().diff(d_iv, d_iv_list);

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
    if(!pt_x.empty())
        vibes::drawPolygon(pt_x, pt_y, "black[#FF00FF]");

    /// **************** DRAW EMPTY **************** ///
    pt_x.clear(); pt_y.clear();
    for(const tuple<int, int, bool> &t:m_oriented_path){
        IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
        if(m_type != VIBES_MAZE_EULERIAN){
            DoorIBEX *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_maze_outer];
            d_iv = d->get_input() | d->get_output();
        }
        else{
            for(size_t i=0; i<m_eulerian_maze->get_maze_outer_fwd().size(); i++){
                DoorIBEX *d= p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_eulerian_maze->get_maze_outer_fwd()[i]];
                IntervalVector d_tmp = d->get_input() | d->get_output();
                d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[m_eulerian_maze->get_maze_outer_bwd()[m_eulerian_maze->get_maze_outer_bwd().size()-1-i]];
                d_tmp &= d->get_input() | d->get_output();
                d_iv |= d_tmp;
            }
        }
        IntervalVector* d_iv_list;
        int nb_vec = p->get_faces()[get<0>(t)][get<1>(t)]->get_position().diff(d_iv, d_iv_list);

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
    if(!pt_x.empty())
        vibes::drawPolygon(pt_x, pt_y, "black[blue]");

    // Draw Cone
    if(m_type!=VIBES_MAZE_EULERIAN)
        show_theta(p, m_maze_inner);
    else
        show_theta(p, m_eulerian_maze->get_maze_outer(0));
}

void VibesMaze::show_maze_outer() const{
    for(PaveIBEX *p:m_subpaving->get_paves()){
        draw_room_outer(p);
    }
    for(PaveIBEX *p:m_subpaving->get_paves_not_bisectable()){
        //        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
        invariant::RoomIBEX *r = p->get_rooms()[m_maze_outer];
        //            if(!r->is_removed()){
        draw_room_outer(p);
        //            }
        if(r->is_empty())
            vibes::drawBox(p->get_position(), "[blue]");
        //        }
    }
}

void VibesMaze::show_maze_inner() const{
    for(invariant::PaveIBEX *p:m_subpaving->get_paves()){
        draw_room_inner(p);
    }
    for(invariant::PaveIBEX *p:m_subpaving->get_paves_not_bisectable()){
        //        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
        invariant::RoomIBEX *r = p->get_rooms()[m_maze_inner];
        //            if(!r->is_removed()){
        draw_room_inner(p);
        //            }
        if(r->is_empty())
            vibes::drawBox(p->get_position(), "[#FF00FF]");
        //        }
    }
}

void VibesMaze::show_eulerian_maze() const{
    for(PaveIBEX *p:m_subpaving->get_paves()){
        draw_room_inner_outer(p);
    }
}

void VibesMaze::show_maze_outer_inner() const{
    for(PaveIBEX *p:m_subpaving->get_paves()){
        invariant::RoomIBEX *r_inner = p->get_rooms()[m_maze_inner];
        invariant::RoomIBEX *r_outer = p->get_rooms()[m_maze_outer];
        if(!r_inner->is_full_union()){
            if(r_outer->is_full_union())
                draw_room_inner(p);
            else
                draw_room_inner_outer(p);
        }
        else{
            draw_room_outer(p);
        }

        if(!r_outer->is_removed() && !r_outer->is_empty()){
            if(r_outer->is_initial_door_input())
                vibes::drawBox(r_outer->get_initial_door_input(), "[#FF8C8C]");
            if(r_outer->is_initial_door_output())
                vibes::drawBox(r_outer->get_initial_door_output(), "[#FF8C8C]");
        }

//        if(!r_inner->is_removed() && !r_inner->is_empty()){
//            if(r_inner->is_initial_door_input())
//                vibes::drawBox(r_inner->get_initial_door_input(), "[#FF8C8C]");
//            if(r_inner->is_initial_door_output())
//                vibes::drawBox(r_inner->get_initial_door_output(), "[#FF8C8C]");
//        }
    }

    for(invariant::PaveIBEX *p:m_subpaving->get_paves_not_bisectable()){
        //        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
        invariant::RoomIBEX *r_outer = p->get_rooms()[m_maze_outer];
        invariant::RoomIBEX *r_inner = p->get_rooms()[m_maze_inner];
        if(r_outer->is_empty())
            vibes::drawBox(p->get_position(), "black[blue]");
        if(r_inner->is_empty())
            vibes::drawBox(p->get_position(), "black[#FF00FF]");
        //        }
    }
}

void VibesMaze::show_theta(invariant::PaveIBEX *p, invariant::MazeIBEX* maze) const{
    IntervalVector position(p->get_position());
    double size = 0.8*min(position[0].diam(), position[1].diam())/2.0;
    invariant::RoomIBEX *r = p->get_rooms()[maze];
//    invariant::DYNAMICS_SENS sens = r->get_maze()->get_dynamics()->get_sens();

    const vector<IntervalVector> vector_fields = r->get_vector_fields();
    IntervalVector empty(2, ibex::Interval::EMPTY_SET);

    if(vector_fields.size()>0){
        for(IntervalVector r:vector_fields){
//            if(sens == invariant::BWD)
//                r=-r;
            if(r == empty)
                vibes::drawBox(position[0].mid()+ibex::Interval(-size, +size), position[1].mid()+ibex::Interval(-size, +size), "black[gray]");
            else{
                for(ibex::Interval i:compute_theta(r[0], r[1])){
                    vibes::drawSector(position[0].mid(), position[1].mid(), size, size, (-i.lb())*180.0/M_PI, (-i.ub())*180.0/M_PI, "black[gray]");
                }
            }
        }
    }
    else{
        vibes::drawSector(position[0].mid(), position[1].mid(), size, size, -180, 180, "black[gray]");
    }

}

std::vector<ibex::Interval> VibesMaze::compute_theta(ibex::Interval dx, ibex::Interval dy) const{
    std::vector<ibex::Interval> theta_list;

    for(int i=0; i<2; i++)
        theta_list.push_back(ibex::Interval::EMPTY_SET);

    ibex::Interval theta = atan2(dy, dx);

    if(theta==(-ibex::Interval::PI|ibex::Interval::PI)){
        ibex::Interval thetaR = atan2(-dy, -dx); // PI rotation ({dx, dy} -> {-dx, -dy})
        if(thetaR.diam()<theta.diam()){
            if(thetaR.is_subset(-ibex::Interval::PI | ibex::Interval::PI)){
                theta_list[0] = (thetaR + ibex::Interval::PI) & (ibex::Interval::ZERO | ibex::Interval::PI); // theta[0] in [0, pi]
                theta_list[1] = ((thetaR + ibex::Interval::PI) & (ibex::Interval::PI | ibex::Interval::TWO_PI)) - ibex::Interval::TWO_PI; // theta[1] in [-pi, 0]
            }
            else
                throw runtime_error("in [VibesMaze.cpp/compute_theta] error with the computation of theta");
        }
        else
            theta_list[0] = theta;
    }
    else if(theta.is_empty())
        theta_list[0] = -ibex::Interval::PI|ibex::Interval::PI;
    else
        theta_list[0] = theta;
    if(theta_list[0].is_empty())
        throw runtime_error("in [VibesMaze.cpp/compute_theta] error with the computation of theta");


    if(theta_list[1].is_empty())
        theta_list.erase(theta_list.begin()+1);

    return theta_list;
}

void VibesMaze::show_graph() const{
    IntervalVector bounding_box(m_subpaving->dim(), ibex::Interval::EMPTY_SET);
    //    vibes::newGroup("graph_bisectable", "gray[gray]", vibesParams("figure", m_name));
    //    vibes::newGroup("graph_not_bisectable", "lightGray[lightGray]", vibesParams("figure", m_name));

    //    vibes::Params params_bisectable, params_not_bisectable;
    //    params_bisectable = vibesParams("figure", m_name, "group", "graph_bisectable", "FaceColor","none","EdgeColor","gray");
    //    params_not_bisectable = vibesParams("figure", m_name, "group", "graph_not_bisectable", "FaceColor","none","EdgeColor","lightGray");

    for(PaveIBEX*p:m_subpaving->get_paves()){
        ibex::IntervalVector box(p->get_position());
        //        vibes::drawBox(box, params_bisectable);
        bounding_box |= box;
    }
    for(PaveIBEX*p:m_subpaving->get_paves_not_bisectable()){
        if(!p->get_position().is_unbounded()){
            //            vibes::drawBox(p->get_position(), params_not_bisectable);
            bounding_box |= p->get_position();
        }
    }

    double overhead_x0 = bounding_box.diam()[0]*m_overhead_factor;
    double overhead_x1 = bounding_box.diam()[1]*m_overhead_factor;
    vibes::axisLimits(bounding_box[0].lb()-overhead_x0, bounding_box[0].ub()+overhead_x0,
            bounding_box[1].lb()-overhead_x1, bounding_box[1].ub()+overhead_x1,
            m_name);
}

void VibesMaze::get_room_info(invariant::MazeIBEX *maze, const ibex::IntervalVector& position) const{
    std::vector<invariant::PaveIBEX*> pave_list;
    m_subpaving->get_room_info(maze, position, pave_list);
    for(invariant::PaveIBEX* p:pave_list){
        vibes::drawCircle(p->get_position()[0].mid(), p->get_position()[1].mid(),
                0.6*min(p->get_position()[0].diam()/2.0, p->get_position()[1].diam()/2.0), "[green]");
    }
}

void VibesMaze::get_room_info(invariant::MazeIBEX *maze, double x, double y) const{
    IntervalVector pos(2);
    pos[0] = ibex::Interval(x);
    pos[1] = ibex::Interval(y);
    get_room_info(maze, pos);
}

void VibesMaze::show_room_info(invariant::MazeIBEX *maze, const IntervalVector& position){
    std::vector<invariant::PaveIBEX*> pave_list;
    std::cout.precision(15);
    m_subpaving->get_room_info(maze, position, pave_list);
    for(invariant::PaveIBEX* p:pave_list){
        vibes::drawCircle(p->get_position()[0].mid(), p->get_position()[1].mid(),
                0.6*min(p->get_position()[0].diam()/2.0, p->get_position()[1].diam()/2.0), "[green]", vibesParams("figure", m_name));
    }

    for(invariant::PaveIBEX* p:pave_list){
        IntervalVector p_position = p->get_position();
        double p_diam[2] = {p_position[0].diam(), p_position[1].diam()};
        double offset[2] = {p_diam[0]*0.05, p_diam[1]*0.05};

        // New Figure
        ostringstream name;
        name << p_position;
        vibes::newFigure(name.str());
        vibes::setFigureProperties(vibesParams("width", m_width,"height", m_height, "x", m_x,"y", m_y));
        //        setProperties(m_x+m_width, m_y, m_width, m_height);

        // Draw Pave
        if(m_maze_inner != nullptr){ //maze->get_domain()->get_init() == FULL_WALL){
            draw_room_inner(p);
        }
        if(m_maze_outer != nullptr){//maze->get_domain()->get_init() == FULL_DOOR){
            draw_room_outer(p);
        }
        //        else{
        //            draw_room_outer(p);
        //            draw_room_inner(p);
        //        }


        // Draw Doors
        for(int face=0; face<2; face++){
            for(int sens=0; sens<2; sens++){
                invariant::FaceIBEX *f=p->get_faces()[face][sens];
                invariant::DoorIBEX *d = f->get_doors()[maze];

                IntervalVector input = d->get_input();
                IntervalVector output = d->get_output();

                input[face] += 1.0*ibex::Interval((sens==1)?(offset[face]):(-offset[face])) + ibex::Interval(-offset[face]/4.0, offset[face]/4.0);
                output[face] += 2.0*ibex::Interval((sens==1)?(offset[face]):(-offset[face])) + ibex::Interval(-offset[face]/4.0, offset[face]/4.0);

                vibes::drawBox(input, "red[red]");
                vibes::drawBox(output, "blue[blue]");
            }
        }
        vibes::axisLimits(p_position[0].lb()-3*offset[0], p_position[0].ub()+3*offset[0],
                p_position[1].lb()-3*offset[1], p_position[1].ub()+3*offset[1]);
    }
}

void VibesMaze::drawCircle(double x_center, double y_center, double radius, string params) const{
    vibes::drawCircle(x_center, y_center, radius, params);
}

void VibesMaze::drawBox(double x_min, double x_max, double y_min, double y_max, std::string params) const{
    vibes::drawBox(x_min, x_max, y_min, y_max, params);
}

void VibesMaze::drawBox(const ibex::IntervalVector &box, std::string params) const{
    vibes::drawBox(box, params);
}

namespace vibes{
inline void drawGraph(const invariant::SmartSubPavingIBEX &g, Params params){
    vibes::drawPave(g.get_paves(), params);
    vibes::drawPave(g.get_paves_not_bisectable(), params);
}

inline void drawPave(const invariant::PaveIBEX &p, Params params){
    vibes::drawBox(p.get_position(), params);
}

inline void drawPave(const vector<invariant::PaveIBEX*>& l_p, Params params){
    for(PaveIBEX *p:l_p)
        vibes::drawPave(*p, params);
}

inline void drawFace(const invariant::FaceIBEX &f, Params params){
    vibes::drawBox(f.get_position(), params);
}

inline void drawFace(const vector<invariant::FaceIBEX *>& l_f, Params params){
    for(invariant::FaceIBEX *f:l_f)
        vibes::drawFace(*f, params);
}
}
