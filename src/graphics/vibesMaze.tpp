#include "vibesMaze.h"
#include <regex>
#include <ostream>

using namespace invariant;
using namespace ibex;
using namespace std;

#undef WITH_IPEGENERATOR
template <typename _TpR, typename _TpF, typename _TpD>
VibesMaze<_TpR,_TpF,_TpD>::VibesMaze(const std::string& figure_name, invariant::SmartSubPaving<_TpR,_TpF,_TpD> *g):
    VibesFigure(figure_name), m_scale_factor(2), m_offset(2)
{
    if(g->dim() != 2)
        throw std::runtime_error("in [vibes_graph.cpp/VibesMaze()] dim of paving is not equal to 2");
    m_subpaving = g;
    m_overhead_factor = 0.0; // 20%

    m_oriented_path.push_back(std::make_tuple(0, 0, true));
    m_oriented_path.push_back(std::make_tuple(1, 1, true));
    m_oriented_path.push_back(std::make_tuple(0, 1, false));
    m_oriented_path.push_back(std::make_tuple(1, 0, false));
    m_scale_factor[0] = ibex::Interval(1);
    m_scale_factor[1] = ibex::Interval(1);
    m_offset[0] = ibex::Interval::ZERO;
    m_offset[1] = ibex::Interval::ZERO;

#ifdef WITH_IPEGENERATOR
    m_ipe_figure = new ipegenerator::Figure(m_subpaving->get_position(), 210, 297, true);
    //    m_ipe_figure->add_layer("outer");
    //    m_ipe_figure->add_layer("inner");
    //    m_ipe_figure->add_layer("uncertain");
    //    m_ipe_figure->set_visible("outer", true);
#endif
}

template <typename _TpR, typename _TpF, typename _TpD>
VibesMaze<_TpR,_TpF,_TpD>::VibesMaze(const std::string& figure_name, invariant::Maze<_TpR,_TpF,_TpD>* maze, VIBES_MAZE_TYPE type): VibesMaze(figure_name, maze->get_subpaving()){
    if(type == VIBES_MAZE_OUTER){
        m_maze_outer.push_back(maze);
        m_type = VIBES_MAZE_OUTER;
    }
    else{
        m_maze_inner.push_back(maze);
        m_type = VIBES_MAZE_INNER;
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
VibesMaze<_TpR,_TpF,_TpD>::VibesMaze(const std::string& figure_name, invariant::Maze<_TpR,_TpF,_TpD>* outer, invariant::Maze<_TpR,_TpF,_TpD>* inner, bool both_wall): VibesMaze(figure_name, outer->get_subpaving()){
    m_maze_outer.push_back(outer);
    m_maze_inner.push_back(inner);
    m_type = VIBES_MAZE_OUTER_AND_INNER;
    m_both_wall = both_wall;
}

template <typename _TpR, typename _TpF, typename _TpD>
VibesMaze<_TpR,_TpF,_TpD>::VibesMaze(const std::string& figure_name, std::vector<invariant::Maze<_TpR,_TpF,_TpD>*> &outer, std::vector<invariant::Maze<_TpR,_TpF,_TpD>*> &inner, bool both_wall): VibesMaze(figure_name, outer[0]->get_subpaving()){
    m_maze_outer = outer;
    m_maze_inner = inner;
    m_type = VIBES_MAZE_OUTER_AND_INNER;
    m_both_wall = both_wall;
}

template <typename _TpR, typename _TpF, typename _TpD>
VibesMaze<_TpR,_TpF,_TpD>::VibesMaze(const std::string& figure_name, invariant::EulerianMaze<_TpR,_TpF,_TpD> *e): VibesMaze(figure_name, e->get_paving()){
    m_eulerian_maze = e;
    m_type = VIBES_MAZE_EULERIAN;
}

template <typename _TpR, typename _TpF, typename _TpD>
VibesMaze<_TpR,_TpF,_TpD>::~VibesMaze(){
#ifdef WITH_IPEGENERATOR
    delete(m_ipe_figure);
#endif
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::drawBox_iv(const ibex::IntervalVector &box, const std::string &color_stroke, const std::string &color_fill) const{
    if(m_enable_show)
        vibes::drawBox(box, concat_color(color_stroke, color_fill));
#ifdef WITH_IPEGENERATOR
    ipe_color_converter(color_stroke, color_fill);
    m_ipe_figure->draw_box(box);
#endif
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show() const{
    this->selectFigure();
    show_graph();
    if(m_eulerian_maze != nullptr)
        show_eulerian_maze();
    else if(m_maze_outer.size()!=0 && m_maze_inner.size()==0)
        show_maze_outer();
    else if(m_maze_outer.size()==0 && m_maze_inner.size()!=0)
        show_maze_inner();
    else if(m_maze_outer.size()!=0 && m_maze_inner.size()!=0)
        show_maze_outer_inner();
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::draw_room_inner(Pave<_TpR,_TpF,_TpD> *p) const{
    // Draw Polygon
    vector<double> pt_x, pt_y;

    for(const tuple<int, int, bool> &t:m_oriented_path){
        IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
        for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_inner){
            invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[maze];
            d_iv |= (d->get_input() | d->get_output());
        }
        IntervalVector* d_iv_list;
        int nb_vec = 1;
        if(m_both_wall)
            d_iv_list = new ibex::IntervalVector(d_iv);
        else
            nb_vec = p->get_faces()[get<0>(t)][get<1>(t)]->get_position().diff(d_iv, d_iv_list);

        if(nb_vec == 1){
            IntervalVector d_iv_c(d_iv_list[0]);
            d_iv_c = hadamard_product(d_iv_c+m_offset,m_scale_factor);
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
        if(m_both_wall)
            delete d_iv_list;
        else
            delete[] d_iv_list;
    }
    if(!pt_x.empty())
        this->drawPolygon(pt_x, pt_y, "black","#FF00FF");

    if(!m_both_wall){
        pt_x.clear(); pt_y.clear();
        for(const tuple<int, int, bool> &pt:m_oriented_path){
            IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_inner){
                invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[maze];
                d_iv |= (d->get_input() | d->get_output());
            }

            if(!d_iv.is_empty()){
                d_iv = hadamard_product(d_iv+m_offset, m_scale_factor);
                if(get<2>(pt)){
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
            this->drawPolygon(pt_x, pt_y, "black","yellow");
    }

    // Draw Cone
    for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_inner)
        show_theta(p, maze);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::draw_room_outer(Pave<_TpR,_TpF,_TpD> *p) const{
    // Draw backward
    this->drawBox_with_scale(p->get_position(), "black","blue");

    // Draw Polygon
    vector<double> pt_x, pt_y;

    bool display_yellow = true;
    for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
        invariant::Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[maze];
        if(!(!r->is_removed() && r->get_contain_zero() && maze->get_contraction_step()==0)){ // Begining issue
            display_yellow = false;
            break;
        }
    }

    if(display_yellow){ // Begining issue
        this->drawBox_with_scale(p->get_position(), "black","yellow");
    }
    else{
        for(const tuple<int, int, bool> &pt:m_oriented_path){
            IntervalVector d_iv(2);
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
                invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[maze];
                d_iv &= (d->get_input() | d->get_output());
            }

            if(!d_iv.is_empty()){
                d_iv = hadamard_product(d_iv+m_offset, m_scale_factor);
                if(get<2>(pt)){
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
            this->drawPolygon(pt_x, pt_y, "black","yellow");
    }

    // Draw Cone
    for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer)
        show_theta(p, maze);

    // Hybrid
    for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
        invariant::Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[maze];
        if(!r->get_hybrid_door_guards().empty()){
            for(std::pair<ibex::Sep*, ibex::IntervalVector> g:r->get_hybrid_door_guards()){
              if(!g.second.is_empty() /*&& !g.second.is_flat()*/)
                this->drawBox_with_scale(g.second, "black","green");
            }
        }
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
double VibesMaze<_TpR,_TpF,_TpD>::get_volume_pave(Pave<_TpR,_TpF,_TpD> *p, const bool inner) const{
    /// ToDo Update for multiple maze
    //    if(p->get_rooms()[(inner)?m_maze_inner:m_maze_outer]->is_full())
    //        return p->get_position().volume();
    //    if(!inner && p->get_rooms()[m_maze_outer]->is_empty())
    //        return 0.;
    //    if(inner && p->get_rooms()[m_maze_outer]->is_empty())
    //        return p->get_position().volume();

    vector<double> pt_x, pt_y;
    for(const tuple<int, int, bool> &t:m_oriented_path){

        IntervalVector d_iv(2);
        if(inner)
            d_iv = IntervalVector(2, ibex::Interval::EMPTY_SET);
        for(invariant::Maze<_TpR,_TpF,_TpD>* maze:(inner)?m_maze_inner:m_maze_outer){
            invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[maze];
            if(!inner)
                d_iv &= (d->get_input() | d->get_output());
            else
                d_iv |= (d->get_input() | d->get_output());
        }

        //        IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
        //        Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(t)][get<1>(t)]->get_doors()[(inner)?m_maze_inner:m_maze_outer];
        //        d_iv = d->get_input() | d->get_output();

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
    if(pt_x.empty())
        return 0.;

    // Clean list
    for(size_t i=0; i<pt_x.size()-1; i++){
        if(pt_x[i]==pt_x[i+1] && pt_y[i]==pt_y[i+1]){
            pt_x.erase(pt_x.begin()+(i+1));
            pt_y.erase(pt_y.begin()+(i+1));
        }
    }
    if(pt_x[0]==pt_x[pt_x.size()-1] && pt_y[0]==pt_y[pt_y.size()-1]){
        pt_x.pop_back();
        pt_y.pop_back();
    }
    pt_x.push_back(pt_x[0]);
    pt_y.push_back(pt_y[0]);

    // Compute Area
    double A=0.;
    for(size_t i=0; i<pt_x.size()-1;i++){
        A+= (pt_x[i]*pt_y[i+1]-pt_x[i+1]*pt_y[i]);
    }
    return abs(0.5*A);
}

template <typename _TpR, typename _TpF, typename _TpD>
double VibesMaze<_TpR,_TpF,_TpD>::get_volume(const bool inner) const{
    if(inner && m_maze_inner.size()==0)
        return 0.;
    double A=0.;
#pragma omp parallel for
    for(size_t i=0; i<m_subpaving->get_paves().size(); i++){
        double a = get_volume_pave(m_subpaving->get_paves()[i],inner);
#pragma omp critical
        {
            A+=a;
        }
    }

#pragma omp parallel for
    for(size_t i=0; i<m_subpaving->get_paves_not_bisectable().size(); i++){
        double a = get_volume_pave(m_subpaving->get_paves_not_bisectable()[i],inner);
#pragma omp critical
        {
            A+=a;
        }
    }

    if(!inner)
        return A;
    else{
        return m_subpaving->get_position().volume() - A;
    }
}


template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::draw_room_inner_outer(Pave<_TpR,_TpF,_TpD> *p) const{
    // Draw backward
    //    if(m_type == VIBES_MAZE_INNER) // Otherwise draw box only when outer
    //        this->drawBox_with_scale(p->get_position(), "black[]");

    /// **************** DRAW UNCERTAIN **************** ///
    vector<double> pt_x, pt_y;
    bool full_outer_eulerian = true;
    for(const tuple<int, int, bool> &pt:m_oriented_path){
        IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
        if(m_type==VIBES_MAZE_EULERIAN){
            for(size_t i=0; i<m_eulerian_maze->get_maze_outer_fwd().size(); i++){
                Door<_TpR,_TpF,_TpD> *d= p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[m_eulerian_maze->get_maze_outer_fwd()[i]];
                IntervalVector d_tmp(2, ibex::Interval::EMPTY_SET);
                d_tmp |= d->get_input() | d->get_output();
                d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[m_eulerian_maze->get_maze_outer_bwd()[m_eulerian_maze->get_maze_outer_bwd().size()-1-i]];
                d_tmp &= d->get_input() | d->get_output();
                d_iv |= d_tmp;
            }
            if(d_iv != p->get_faces()[get<0>(pt)][get<1>(pt)]->get_position())
                full_outer_eulerian = false;
        }
        else{
            d_iv = IntervalVector(2, ibex::Interval::ALL_REALS);
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
                invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[maze];
                d_iv &= (d->get_input() | d->get_output());
            }

            // *****************
            // Test
            IntervalVector d_iv_inner = IntervalVector(2, ibex::Interval::EMPTY_SET);
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_inner){
                invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[maze];
                d_iv_inner |= (d->get_input() | d->get_output());
            }
            d_iv &= d_iv_inner;
            // *****************
        }

        if(!d_iv.is_empty()){
            d_iv = hadamard_product(d_iv+m_offset, m_scale_factor);
            if(get<2>(pt)){
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

    if(m_type==VIBES_MAZE_EULERIAN && !full_outer_eulerian)
        this->drawBox_with_scale(p->get_position(), "black","blue");
    if(m_export_uncertain && !pt_x.empty())
        this->drawPolygon(pt_x, pt_y, "black","yellow");


    /// **************** DRAW INNER **************** ///
    pt_x.clear(); pt_y.clear();
    for(const tuple<int, int, bool> &pt:m_oriented_path){
        IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
        if(m_type==VIBES_MAZE_EULERIAN){
            d_iv = IntervalVector(2, ibex::Interval::ALL_REALS);
            for(size_t i=0; i<m_eulerian_maze->get_maze_inner_fwd().size(); i++){
                Door<_TpR,_TpF,_TpD> *d= p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[m_eulerian_maze->get_maze_inner_fwd()[i]];
                IntervalVector d_tmp(2, ibex::Interval::EMPTY_SET);
                d_tmp |= d->get_input() | d->get_output();
                d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[m_eulerian_maze->get_maze_inner_bwd()[m_eulerian_maze->get_maze_inner_bwd().size()-1-i]];
                d_tmp |= d->get_input() | d->get_output();
                d_iv &= d_tmp;
            }
        }
        else{
            d_iv = IntervalVector(2, ibex::Interval::EMPTY_SET);
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_inner){
                invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[maze];
                d_iv |= (d->get_input() | d->get_output());
            }
        }

        IntervalVector* d_iv_list;
        int nb_vec = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_position().diff(d_iv, d_iv_list);

        if(nb_vec == 1){
            IntervalVector d_iv_c(d_iv_list[0]);
            d_iv_c = hadamard_product(d_iv_c+m_offset, m_scale_factor);
            if(!d_iv_c.is_empty()){
                if(get<2>(pt)){
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
    if(m_export_inner && !pt_x.empty())
        this->drawPolygon(pt_x, pt_y, "black","#FF00FF");

    /// **************** DRAW OUTER **************** ///
    bool full_outer = true;
    for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
        if(!p->get_rooms()[maze]->is_full()){
            full_outer = false;
            break;
        }
    }

    if(!full_outer){
        pt_x.clear(); pt_y.clear();
        for(const tuple<int, int, bool> &pt:m_oriented_path){
            IntervalVector d_iv(2, ibex::Interval::EMPTY_SET);
            if(m_type != VIBES_MAZE_EULERIAN){
                d_iv = IntervalVector(2, ibex::Interval::ALL_REALS);
                for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
                    invariant::Door<_TpR,_TpF,_TpD> *d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[maze];
                    d_iv &= (d->get_input() | d->get_output());
                }
            }
            else{
                for(size_t i=0; i<m_eulerian_maze->get_maze_outer_fwd().size(); i++){
                    Door<_TpR,_TpF,_TpD> *d= p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[m_eulerian_maze->get_maze_outer_fwd()[i]];
                    IntervalVector d_tmp(2, ibex::Interval::EMPTY_SET);
                    d_tmp |= d->get_input() | d->get_output();
                    d = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_doors()[m_eulerian_maze->get_maze_outer_bwd()[m_eulerian_maze->get_maze_outer_bwd().size()-1-i]];
                    d_tmp &= d->get_input() | d->get_output();
                    d_iv |= d_tmp;
                }
            }
            IntervalVector* d_iv_list;
            int nb_vec = p->get_faces()[get<0>(pt)][get<1>(pt)]->get_position().diff(d_iv, d_iv_list);

            if(nb_vec == 1){
                IntervalVector d_iv_c(d_iv_list[0]);
                d_iv_c = hadamard_product(d_iv_c+m_offset, m_scale_factor);
                if(!d_iv_c.is_empty()){
                    if(get<2>(pt)){
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
        if(m_export_outer && !pt_x.empty())
            this->drawPolygon(pt_x, pt_y, "black","blue");
    }

    // Draw Cone
    if(m_enable_cones){
        if(m_type!=VIBES_MAZE_EULERIAN){
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer)
                show_theta(p, maze);
        }
        else
            show_theta(p, m_eulerian_maze->get_maze_outer(0));
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show_maze_outer() const{
    for(Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves()){
        draw_room_outer(p);
    }
    for(Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves_not_bisectable()){
        //        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
        //        invariant::Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[m_maze_outer];
        //            if(!r->is_removed()){
        draw_room_outer(p);
        //            }
        //        if(r->is_empty())
        //            this->drawBox_with_scale(p->get_position(), "[blue]");
        //        }
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show_maze_inner() const{
    for(invariant::Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves()){
        draw_room_inner(p);
    }
    for(invariant::Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves_not_bisectable()){
        //        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !
        //        invariant::Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[m_maze_inner];
        //            if(!r->is_removed()){
        draw_room_inner(p);
        //            }
        //        if(r->is_empty())
        //            this->drawBox_with_scale(p->get_position(), "[#FF00FF]");
        //        }
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show_eulerian_maze() const{
    for(Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves()){
        draw_room_inner_outer(p);
    }
    //    for(Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves_not_bisectable()){
    //        draw_room_outer(p);
    //    }
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show_maze_outer_inner() const{
    for(Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves()){
        draw_room_inner_outer(p);

        for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
            invariant::Room<_TpR,_TpF,_TpD> *r_outer = p->get_rooms()[maze];
            if(!r_outer->is_removed() && !r_outer->is_empty()){
                if(r_outer->is_initial_door_input())
                    this->drawBox_with_scale(r_outer->get_initial_door_input(), "", "#FF8C8C");
                if(r_outer->is_initial_door_output())
                    this->drawBox_with_scale(r_outer->get_initial_door_output(), "","#FF8C8C");
            }
        }
    }

    for(invariant::Pave<_TpR,_TpF,_TpD> *p:m_subpaving->get_paves_not_bisectable()){
        //        if(!p->is_infinite()){ /// ToDo : change if implementing infinite paves !

        if(m_export_outer){
            bool empty_outer = false;
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_outer){
                invariant::Room<_TpR,_TpF,_TpD> *r_outer = p->get_rooms()[maze];
                if(r_outer->is_empty()){
                    empty_outer = true;
                    break;
                }
            }

            if(empty_outer)
                this->drawBox_with_scale(p->get_position(), "black","blue");
        }

        if(m_export_inner && !m_both_wall){
            bool empty_inner = true;
            for(invariant::Maze<_TpR,_TpF,_TpD>* maze:m_maze_inner){
                invariant::Room<_TpR,_TpF,_TpD> *r_inner = p->get_rooms()[maze];
                if(!r_inner->is_empty()){
                    empty_inner = false;
                    break;
                }
            }

            if(empty_inner)
                this->drawBox_with_scale(p->get_position(), "black","#FF00FF");
        }
        //        }
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show_theta(invariant::Pave<_TpR,_TpF,_TpD> *p, invariant::Maze<_TpR,_TpF,_TpD>* maze) const{
    if(!m_enable_cones)
        return;
    IntervalVector position(hadamard_product(p->get_position()+m_offset, m_scale_factor));
    //    double size = 0.8*sqrt(pow(position[0].diam()/2.,2)+ pow(position[1].diam()/2., 2));
    double size = 0.8*min(position[0].diam(), position[1].diam())/2.;
    //    cout << "size = " << size << endl;
    invariant::Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[maze];
    //    invariant::DYNAMICS_SENS sens = r->get_maze()->get_dynamics()->get_sens();

    const vector<IntervalVector> vector_fields = r->get_vector_fields();
    IntervalVector empty(2, ibex::Interval::EMPTY_SET);

    if(vector_fields.size()>0){
        for(IntervalVector r:vector_fields){
            if(r == empty)
                this->drawBox(position.mid()+ibex::IntervalVector(2,ibex::Interval(-size, size)), "black","gray");
            else{
                for(ibex::Interval i:compute_theta((r[0]+m_offset[0])*m_scale_factor[0], (r[1]+m_offset[1])*m_scale_factor[1])){
                    this->drawSector(position[0].mid(), position[1].mid(), size, size, (-i.lb())*180.0/M_PI, (-i.ub())*180.0/M_PI, "black","#8080807F");
                }
            }
            size*=1.1;
        }
    }
    else{
        this->drawSector(position[0].mid(), position[1].mid(), size, size, -180, 180, "black","gray");
    }

}

template <typename _TpR, typename _TpF, typename _TpD>
std::vector<ibex::Interval> VibesMaze<_TpR,_TpF,_TpD>::compute_theta(ibex::Interval dx, ibex::Interval dy) const{
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

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show_graph() const{
    IntervalVector bounding_box(m_subpaving->dim(), ibex::Interval::EMPTY_SET);

    for(Pave<_TpR,_TpF,_TpD>*p:m_subpaving->get_paves()){
        bounding_box |= hadamard_product(p->get_position()+m_offset,m_scale_factor);

    }
    for(Pave<_TpR,_TpF,_TpD>*p:m_subpaving->get_paves_not_bisectable()){
        if(!p->get_position().is_unbounded())
            bounding_box |= hadamard_product(p->get_position()+m_offset,m_scale_factor);
    }

    if(m_enable_white_boundary)
        this->drawBox(bounding_box, "white","");
    bounding_box += m_overhead_factor*bounding_box.diam();
    this->axisLimits(bounding_box);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::get_room_info(invariant::Maze<_TpR,_TpF,_TpD> *maze, const ibex::IntervalVector& position) const{
    std::vector<invariant::Pave<_TpR,_TpF,_TpD>*> pave_list;
    m_subpaving->get_room_info(maze, position, pave_list);
    for(invariant::Pave<_TpR,_TpF,_TpD>* p:pave_list){
        this->drawCircle(p->get_position()[0].mid(), p->get_position()[1].mid(),
                0.6*min(p->get_position()[0].diam()/2.0, p->get_position()[1].diam()/2.0), "", "green");
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::get_room_info(invariant::Maze<_TpR,_TpF,_TpD> *maze, double x, double y) const{
    IntervalVector pos(2);
    pos[0] = ibex::Interval(x);
    pos[1] = ibex::Interval(y);
    get_room_info(maze, pos);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::show_room_info(invariant::Maze<_TpR,_TpF,_TpD> *maze, const IntervalVector& position){
    std::vector<invariant::Pave<_TpR,_TpF,_TpD>*> pave_list;
    std::cout.precision(15);
    m_subpaving->get_room_info(maze, position, pave_list);
    for(invariant::Pave<_TpR,_TpF,_TpD>* p:pave_list){
        ibex::IntervalVector p_tmp = hadamard_product(p->get_position()+m_offset, m_scale_factor);
        this->drawEllipse(p_tmp[0].mid(), p_tmp[1].mid(),
                0.6*p_tmp[0].diam()/2.0, 0.6*p_tmp[1].diam()/2.0, 0., "","green"/*, vibesParams("figure", m_name)*/);
    }

    for(invariant::Pave<_TpR,_TpF,_TpD>* p:pave_list){
        IntervalVector p_position = hadamard_product(p->get_position()+m_offset, m_scale_factor);
        double p_diam[2] = {p_position[0].diam(), p_position[1].diam()};
        double offset[2] = {p_diam[0]*0.05, p_diam[1]*0.05};

        // New Figure
        ostringstream name;
        name << p_position;
        vibes::newFigure(name.str());
        vibes::setFigureProperties(vibesParams("width", m_width,"height", m_height, "x", m_x+100,"y", m_y+100));
        //        setProperties(m_x+m_width, m_y, m_width, m_height);

        // Draw Pave
        if(m_maze_inner.size()!=0 && maze == m_maze_inner[0])
            draw_room_inner(p);
        else if(m_maze_outer.size()!=0){//maze->get_domain()->get_init() == FULL_DOOR){
            draw_room_outer(p);
        }
        //        else{
        //            draw_room_outer(p);
        //            draw_room_inner(p);
        //        }


        // Draw Doors
        for(int face=0; face<2; face++){
            for(int sens=0; sens<2; sens++){
                invariant::Face<_TpR,_TpF,_TpD> *f=p->get_faces()[face][sens];
                invariant::Door<_TpR,_TpF,_TpD> *d = f->get_doors()[maze];

                IntervalVector input = hadamard_product(d->get_input()+m_offset, m_scale_factor);
                IntervalVector output = hadamard_product(d->get_output()+m_offset, m_scale_factor);

                input[face] += 1.0*ibex::Interval((sens==1)?(offset[face]):(-offset[face])) + ibex::Interval(-offset[face]/4.0, offset[face]/4.0);
                output[face] += 2.0*ibex::Interval((sens==1)?(offset[face]):(-offset[face])) + ibex::Interval(-offset[face]/4.0, offset[face]/4.0);

                this->drawBox(input, "red","red");
                this->drawBox(output, "blue","blue");
            }
        }
        ibex::IntervalVector bounding_box(p_position);
        ibex::IntervalVector offset_iv(2);
        offset_iv[0] = 3*position[0]*ibex::Interval(-1,1);
        offset_iv[0] = 3*position[1]*ibex::Interval(-1,1);
        bounding_box += offset_iv;
        this->axisLimits(bounding_box);
    }
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::drawCircle(const double x_center, const double y_center, const double radius, const std::string &color_stroke, const std::string &color_fill) const{
    if(m_enable_show)
        vibes::drawCircle(x_center, y_center, radius, concat_color(color_stroke, color_fill));

#ifdef WITH_IPEGENERATOR
    ipe_color_converter(color_stroke, color_fill);
    m_ipe_figure->draw_circle(x_center, y_center, radius);
#endif
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::drawEllipse(const double &cx, const double &cy, const double &a, const double &b, const double &rot, const std::string &color_stroke, const std::string &color_fill) const{
    if(m_enable_show)
        vibes::drawEllipse(cx, cy, a, b, rot, concat_color(color_stroke, color_fill));
#ifdef WITH_IPEGENERATOR
    ipe_color_converter(color_stroke, color_fill);
    m_ipe_figure->draw_ellipse(cx, cy, a, b);
#endif
}

#ifdef WITH_IPEGENERATOR
template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::ipe_color_converter(const std::string &color_stroke, const std::string &color_fill) const{
    std::string color_stroke_tmp(color_stroke), color_fill_tmp(color_fill);
    if(color_stroke_tmp=="#FF00FF")
        color_stroke_tmp="magenta";
//    if(color_fill_tmp=="#FF8C8C"){
//        color_fill_tmp="pink";
//        m_ipe_figure->set_current_layer("initial_condition");
//    }
    if(color_fill_tmp=="#FF00FF"){
        color_fill_tmp="magenta";
        m_ipe_figure->set_current_layer("inner");
    }
    else if(color_fill_tmp=="blue"){
        m_ipe_figure->set_current_layer("outer");
    }
    else if(color_fill_tmp=="yellow"){
        m_ipe_figure->set_current_layer("uncertain");
    }


    if(color_stroke_tmp!="" && color_fill_tmp!="")
        m_ipe_figure->set_color_type(ipegenerator::STROKE_AND_FILL);
    else if(color_stroke_tmp=="")
        m_ipe_figure->set_color_type(ipegenerator::FILL_ONLY);
    else if(color_fill_tmp=="")
        m_ipe_figure->set_color_type(ipegenerator::STROKE_ONLY);

    if(color_fill_tmp!="")
        m_ipe_figure->set_color_fill(color_fill_tmp);

    if(color_stroke_tmp!="")
        m_ipe_figure->set_color_stroke(color_stroke_tmp);
}
#endif

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::drawPolygon(const std::vector<double> &x, const std::vector<double> &y, const std::string &color_stroke, const std::string &color_fill) const{
    if(m_enable_show)
        vibes::drawPolygon(x, y, concat_color(color_stroke, color_fill));
#ifdef WITH_IPEGENERATOR
    ipe_color_converter(color_stroke, color_fill);
    m_ipe_figure->draw_polygon(x, y);
#endif
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::drawSector(const double x, const double y, const double s_x, const double s_y, const double theta_min, const double theta_max, const std::string &color_stroke, const std::string &color_fill) const{
    if(m_enable_show)
        vibes::drawSector(x, y, s_x, s_y, theta_min, theta_max, concat_color(color_stroke, color_fill));
#ifdef WITH_IPEGENERATOR
    ipe_color_converter(color_stroke, color_fill);
    m_ipe_figure->draw_sector(x, y, s_x, s_y, theta_min, theta_max);
#endif
}

template <typename _TpR, typename _TpF, typename _TpD>
std::string VibesMaze<_TpR,_TpF,_TpD>::concat_color(const std::string &color_stroke, const std::string &color_fill) const{
    return color_stroke + "[" + color_fill + "]";
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::selectFigure() const{
    if(m_enable_show)
        vibes::selectFigure(m_name);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::axisLimits(const ibex::IntervalVector &box) const{
    if(m_enable_show)
        vibes::axisLimits(box[0].lb(), box[0].ub(), box[1].lb(), box[1].ub(),m_name);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::add_stat(size_t step, double time, double volume_outer, double volume_inner){
    m_memory_step.push_back(step);
    m_memory_time.push_back(time);
    m_memory_volume_outer.push_back(volume_outer);
    m_memory_volume_inner.push_back(volume_inner);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::save_stat_to_file(string namefile){
    ofstream stat_file;
    stat_file.open(namefile);
    for(double &d:m_memory_step)
        stat_file << d << " ";
    stat_file << endl;
    for(double &d:m_memory_time)
        stat_file << d << " ";
    stat_file << endl;
    for(double &d:m_memory_volume_outer)
        stat_file << d << " ";
    stat_file << endl;
    for(double &d:m_memory_volume_inner)
        stat_file << d << " ";
    stat_file << endl;
    stat_file.close();
}

#ifdef WITH_IPEGENERATOR
template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::saveIpe(const std::string& prefix) const{
    std::cout << "Saved to : " << prefix << m_name << ".ipe" << std::endl;
    m_ipe_figure->save_ipe(prefix+m_name+".ipe");
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::savePdf(const std::string& prefix) const{
    std::cout << "Saved to : " << prefix << m_name << ".ipe" << std::endl;
    m_ipe_figure->save_pdf(prefix+m_name+".ipe");
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::set_axis_limits(const double start_x, const double inter_x, const double start_y, const double inter_y){
    m_ipe_figure->set_graduation_parameters(start_x, inter_x, start_y, inter_y);
}
template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::draw_axis(const std::string &name_x, const std::string &name_y){
    m_ipe_figure->draw_axis(name_x, name_y);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::draw_text(const std::string &text, const double x, const double y){
    m_ipe_figure->draw_text(text, x, y);
}

template <typename _TpR, typename _TpF, typename _TpD>
void VibesMaze<_TpR,_TpF,_TpD>::set_current_layer(const std::string &layer_name){
    m_ipe_figure->set_current_layer(layer_name);
}

#endif

//namespace vibes{
//inline void drawGraph(const invariant::SmartSubPavingIBEX &g, Params params){
//    vibes::drawPave(g.get_paves(), params);
//    vibes::drawPave(g.get_paves_not_bisectable(), params);
//}

//inline void drawPave(const invariant::PaveIBEX &p, Params params){
//    vibes::drawBox(p.get_position(), params);
//}

//inline void drawPave(const vector<invariant::PaveIBEX*>& l_p, Params params){
//    for(PaveIBEX *p:l_p)
//        vibes::drawPave(*p, params);
//}

//inline void drawFace(const invariant::FaceIBEX &f, Params params){
//    vibes::drawBox(f.get_position(), params);
//}

//inline void drawFace(const vector<invariant::FaceIBEX *>& l_f, Params params){
//    for(invariant::FaceIBEX *f:l_f)
//        vibes::drawFace(*f, params);
//}
//}
