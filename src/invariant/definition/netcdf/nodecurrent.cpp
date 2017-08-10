#include "nodecurrent.h"

using namespace ibex;
using namespace std;

NodeCurrent::NodeCurrent(const ibex::IntervalVector &position, double epsilon_bisection):
    m_position(position), m_vector_field(position.size(), Interval::EMPTY_SET)
{
    if(m_position.max_diam()<epsilon_bisection){
        m_leaf = true;
        m_leaf_list.push_back(this);
    }
    else{
        m_leaf = false;
        ibex::LargestFirst bisector(0, 0.5);
        std::pair<IntervalVector, IntervalVector> result_boxes = bisector.bisect(position);
        NodeCurrent *nc1 = new NodeCurrent(result_boxes.first, epsilon_bisection);
        NodeCurrent *nc2 = new NodeCurrent(result_boxes.second, epsilon_bisection);
        m_children.first = nc1;
        m_children.second = nc2;
        m_leaf_list.insert(m_leaf_list.end(), nc1->get_leaf_list().begin(), nc1->get_leaf_list().end());
        m_leaf_list.insert(m_leaf_list.end(), nc2->get_leaf_list().begin(), nc2->get_leaf_list().end());
    }
}

const ibex::IntervalVector NodeCurrent::compute_vector_field_tree(){
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

const ibex::IntervalVector NodeCurrent::eval(const IntervalVector& position){
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

void NodeCurrent::fill_leafs(const vector<vector<short>> &raw_u, const vector<vector<short>> &raw_v, const float& scale_factor, const short& fill_value){

    int nb_node=m_leaf_list.size();
    size_t dim = m_position.size();

    int d_max[2] = {(int)raw_u.size()-1, (int)raw_u[0].size()-1};

//    #pragma omp parallel for
    for(int id=0; id<nb_node; id++){
        NodeCurrent *nc = m_leaf_list[id];

        vector<vector<int>> tab_point;
        for(size_t d = 0; d<dim; d++){
            vector<int> pt;
            pt.push_back(std::max(0, std::min((int)ceil(nc->get_position()[d].mid()), d_max[d])));
            pt.push_back(std::max(0, std::min((int)floor(nc->get_position()[d].mid()), d_max[d])));
            tab_point.push_back(pt);
        }

        IntervalVector vector_field(dim, Interval::EMPTY_SET);
        bool no_value = false;
        // U & V
        for(size_t k=0; k<tab_point[0].size(); k++){
            for(size_t l=0; l<tab_point[0].size(); l++){
                size_t i_coord = tab_point[0][k];
                size_t j_coord = tab_point[1][l];
                short vec_u = raw_u[i_coord][j_coord];
                short vec_v = raw_v[i_coord][j_coord];
                if(vec_u!=fill_value && vec_v!=fill_value){
                    vector_field[0] |= Interval(vec_u*scale_factor);
                    vector_field[1] |= Interval(vec_v*scale_factor);
                }
                else
                    no_value = true;
            }
        }

        if(no_value)
            vector_field = IntervalVector(2, Interval::EMPTY_SET);

        nc->set_vector_field(vector_field);
    }
}
