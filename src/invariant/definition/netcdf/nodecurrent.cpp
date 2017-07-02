#include "nodecurrent.h"

using namespace ibex;
using namespace std;

NodeCurrent::NodeCurrent(const ibex::IntervalVector &position, double epsilon_bisection):
    m_position(position), m_vector_field(position.size(), Interval::EMPTY_SET)
{
    if(m_position.max_diam()<epsilon_bisection){
        m_leaf = true;
    }
    else{
        m_leaf = false;
        ibex::LargestFirst bisector(0, 0.5);
        std::pair<IntervalVector, IntervalVector> result_boxes = bisector.bisect(position);
        NodeCurrent *nc1 = new NodeCurrent(result_boxes.first, epsilon_bisection);
        NodeCurrent *nc2 = new NodeCurrent(result_boxes.second, epsilon_bisection);
        m_children.first = nc1;
        m_children.second = nc2;
    }
}

const ibex::IntervalVector& NodeCurrent::compute_vector_field_tree(){
    if(m_leaf)
        return m_vector_field;
    else{
        m_vector_field = m_children.first->compute_vector_field_tree();
        m_vector_field |= m_children.second->compute_vector_field_tree();
        return m_vector_field;
    }
}

const ibex::IntervalVector NodeCurrent::eval(const IntervalVector& position){
    if(m_leaf){
        return m_vector_field;
    }
    else{
        IntervalVector inter = position & m_position;
        if(inter.is_empty()){
            return IntervalVector(position.size(), Interval::EMPTY_SET);
        }
//        else if(inter.is_strict_interior_subset(m_position)){
//            return m_vector_field;
//        }
        else{
            IntervalVector result(m_children.first->eval(inter));
            result |= m_children.second->eval(inter);
            return result;
        }
    }
}

void NodeCurrent::fill_leafs(short *raw_u, short *raw_v, const size_t& j_max, const float& scale_factor, const short& fill_value){
    if(m_leaf){
        size_t dim = m_position.size();
        vector<vector<int>> tab_point;
        for(size_t d = 0; d<dim; d++){
            vector<int> pt;
            pt.push_back(ceil(m_position[d].mid()));
            pt.push_back(floor(m_position[d].mid()));
            tab_point.push_back(pt);
        }

        // U
        for(size_t k=0; k<tab_point[0].size(); k++){
            for(size_t l=0; l<tab_point[0].size(); l++){
                size_t j_coord = tab_point[0][k];
                size_t i_coord = tab_point[1][l];
                short v = raw_u[j_coord+i_coord*j_max];
                if(v!=fill_value){
                    m_vector_field[0] |= Interval(v*scale_factor);
                }
            }
        }

        // V
        for(size_t k=0; k<tab_point[0].size(); k++){
            for(size_t l=0; l<tab_point[0].size(); l++){
                int j_coord = tab_point[0][k];
                int i_coord = tab_point[1][l];
                short v = raw_v[j_coord+i_coord*j_max];
                if(v!=fill_value){
                    m_vector_field[1] |= Interval(v*scale_factor);
                }
            }
        }
    }
    else{
        m_children.first->fill_leafs(raw_u, raw_v, j_max, scale_factor, fill_value);
        m_children.second->fill_leafs(raw_u, raw_v, j_max, scale_factor, fill_value);
    }
}
