#include "nodecurrent3d.h"

using namespace ibex;
using namespace std;

namespace invariant {

NodeCurrent3D::NodeCurrent3D(const IntervalVector &position, const std::vector<double> &limit_bisection, PreviMer3D *previmer, std::vector<NodeCurrent3D *> &leaf_list):
    m_position(position), m_vector_field(2, Interval::EMPTY_SET)
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

    if(limit_reach){
        m_leaf = true;
        leaf_list.push_back(this);
    }
    else{
        m_leaf = false;
        std::pair<IntervalVector, IntervalVector> result_boxes = previmer->bisect_largest_first(position);
        NodeCurrent3D *nc1 = new NodeCurrent3D(result_boxes.first, limit_bisection, previmer, leaf_list);
        NodeCurrent3D *nc2 = new NodeCurrent3D(result_boxes.second, limit_bisection, previmer, leaf_list);
        m_children.first = nc1;
        m_children.second = nc2;
    }
}

NodeCurrent3D::~NodeCurrent3D(){
    if(!m_leaf){
        delete(m_children.first);
        delete(m_children.second);
    }
}

const ibex::IntervalVector& NodeCurrent3D::compute_vector_field_tree(){
    if(m_leaf)
        return m_vector_field;
    else{
        m_vector_field = m_children.first->compute_vector_field_tree();
        m_vector_field |= m_children.second->compute_vector_field_tree();
        return m_vector_field;
    }
}

static ibex::IntervalVector empty3(3, ibex::Interval::EMPTY_SET);

const ibex::IntervalVector NodeCurrent3D::eval(const IntervalVector& position){
    if(m_leaf){
        return m_vector_field;
    }
    else{
        IntervalVector inter = position & m_position;
        if(inter.is_empty()){
            return empty3;
        }
        else if(m_position.is_subset(position)){
            return m_vector_field;
        }
        else{
            return (m_children.first->eval(inter) | m_children.second->eval(inter));
        }
    }
}

}

