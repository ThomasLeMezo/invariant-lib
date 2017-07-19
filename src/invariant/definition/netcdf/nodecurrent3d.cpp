#include "nodecurrent3d.h"

using namespace ibex;
using namespace std;

namespace invariant {

NodeCurrent3D::NodeCurrent3D(const IntervalVector &position, const std::vector<double> &limit_bisection, PreviMer3D *previmer, int level, int stop_level):
    m_position(position), m_vector_field(position.size(), Interval::EMPTY_SET)
{
    m_previmer = previmer;
    bool limit_reach = true;
    for(size_t dim=0; dim<3; dim++){
        if(m_position[dim].diam()>limit_bisection[dim]){
            limit_reach = false;
            //            cout << "break " << level << " " << dim << " " << m_position[dim].diam() << " " << limit_bisection[dim] << endl;
            break;
        }
    }
    //    if(level>stop_level)
    //        limit_reach = true;

    if(limit_reach){
        m_leaf = true;
        m_leaf_list.push_back(this);
    }
    else{
        m_leaf = false;
        // ToDo : improve bisection to match change made in pave !!
        std::pair<IntervalVector, IntervalVector> result_boxes = previmer->bisect_largest_first(position);
        NodeCurrent3D *nc1 = new NodeCurrent3D(result_boxes.first, limit_bisection, previmer, level+1, stop_level);
        NodeCurrent3D *nc2 = new NodeCurrent3D(result_boxes.second, limit_bisection, previmer, level+1, stop_level);
        m_children.first = nc1;
        m_children.second = nc2;
        m_leaf_list.insert(m_leaf_list.end(), nc1->get_leaf_list().begin(), nc1->get_leaf_list().end());
        m_leaf_list.insert(m_leaf_list.end(), nc2->get_leaf_list().begin(), nc2->get_leaf_list().end());
    }
}

const ibex::IntervalVector NodeCurrent3D::compute_vector_field_tree(){
    if(m_leaf)
        return m_vector_field;
    else{
        IntervalVector result(m_position.size(), Interval::EMPTY_SET);
        result = m_children.first->compute_vector_field_tree();
        result |= m_children.second->compute_vector_field_tree();
        m_vector_field = result;
        return result;
    }
}

const ibex::IntervalVector NodeCurrent3D::eval(const IntervalVector& position){
    if(m_leaf){
        return m_vector_field;
    }
    else{
        IntervalVector inter = position & m_position;
        if(inter.is_empty()){
            IntervalVector empty(position.size(), Interval::EMPTY_SET);
            return empty;
        }
        else if(m_position.is_subset(position)){
            return m_vector_field;
        }
        else{
            IntervalVector result(position.size(), Interval::EMPTY_SET);
            result |= m_children.first->eval(inter);
            result |= m_children.second->eval(inter);
            return result;
        }
    }
}

void NodeCurrent3D::fill_leafs(const vector<vector<vector<short>>> &raw_u, const vector<vector<vector<short>>> &raw_v, const float& scale_factor, const short& fill_value){

    int nb_node=m_leaf_list.size();
    int d_max[3] = {(int)(raw_u.size())-1, (int)(raw_u[0].size())-1, (int)(raw_u[0][0].size())-1};

    vector<double> grid_size = m_previmer->get_grid_size();

#pragma omp parallel for
    for(int id=0; id<nb_node; id++){
        NodeCurrent3D *nc = m_leaf_list[id];

        /// **** Interpolation of vector field ****
        vector<vector<int>> tab_point_u, tab_point_v;

        for(int dim = 0; dim<3; dim++){
            vector<int> pt_u, pt_v;
            Interval center_u = nc->get_position()[dim].mid() / grid_size[dim]; // Back to the grid coord
            Interval center_v;
            if(dim > 0)
                center_v = (nc->get_position()[dim].mid()+0.5*grid_size[dim]) / grid_size[dim]; // Back to the grid coord
            else
                center_v = nc->get_position()[dim].mid() / grid_size[dim]; // Back to the grid coord

            // Cross pattern
            for(int i=std::max(0, std::min((int)floor(center_u.lb()), d_max[dim]));
                i<std::max(0, std::min((int)ceil(center_u.ub()), d_max[dim]));
                i++){
                pt_u.push_back(i);
            }
            for(int i=std::max(0, std::min((int)floor(center_v.lb()), d_max[dim]));
                i<std::max(0, std::min((int)ceil(center_v.ub()), d_max[dim]));
                i++){
                pt_v.push_back(i);
            }
            tab_point_u.push_back(pt_u);
            tab_point_v.push_back(pt_v);
        }

        IntervalVector vector_field(3, Interval::EMPTY_SET); // 3 Dimensions (dt, U, V)
        // T
//        vector_field[0] = Interval(0.9, 1.1);
        vector_field[0] = Interval(1.0);
        bool no_value = false;
        // U
        for(size_t t_id=0; t_id<tab_point_u[0].size(); t_id++){
            for(size_t u_id=0; u_id<tab_point_u[0].size(); u_id++){
                for(size_t v_id=0; v_id<tab_point_u[0].size(); v_id++){
                    size_t t_coord = tab_point_u[0][t_id];
                    size_t i_coord = tab_point_u[1][u_id];
                    size_t j_coord = tab_point_u[2][v_id];

                    short vec_u = raw_u[t_coord][i_coord][j_coord];
                    if(vec_u!=fill_value){
                        vector_field[1] |= Interval(vec_u*scale_factor);
                    }
                    else
                        no_value = true;
                }
            }
        }
        // V
        for(size_t t_id=0; t_id<tab_point_v[0].size(); t_id++){
            for(size_t u_id=0; u_id<tab_point_v[0].size(); u_id++){
                for(size_t v_id=0; v_id<tab_point_v[0].size(); v_id++){
                    size_t t_coord = tab_point_v[0][t_id];
                    size_t i_coord = tab_point_v[1][u_id];
                    size_t j_coord = tab_point_v[2][v_id];

                    short vec_v = raw_v[t_coord][i_coord][j_coord];
                    if(vec_v!=fill_value){
                        vector_field[2] |= Interval(vec_v*scale_factor);
                    }
                    else
                        no_value = true;
                }
            }
        }

        if(no_value)
            vector_field = IntervalVector(3, Interval::EMPTY_SET);

        nc->set_vector_field(vector_field);
    }
}

}

