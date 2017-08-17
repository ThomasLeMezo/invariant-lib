#include "nodecurrent3d.h"

using namespace ibex;
using namespace std;

namespace invariant {

NodeCurrent3D::NodeCurrent3D(const IntervalVector &position, const std::vector<double> &limit_bisection, PreviMer3D *previmer, std::vector<NodeCurrent3D *> &leaf_list, std::vector<IntervalVector> &leaf_position)
{
    bool limit_reach = true;
    for(size_t dim=0; dim<3; dim++){
        if(position[dim].diam()>limit_bisection[dim]){
            limit_reach = false;
            //            cout << "break " << level << " " << dim << " " << m_position[dim].diam() << " " << limit_bisection[dim] << endl;
            break;
        }
    }

    if(limit_reach){
        m_bisection_axis = -1;
        leaf_list.push_back(this);
        leaf_position.push_back(position);
    }
    else{
        IntervalVector p1(3), p2(3);
        m_bisection_axis = previmer->bisect_largest_first(position, p1, p2);
        NodeCurrent3D *nc1 = new NodeCurrent3D(p1, limit_bisection, previmer, leaf_list, leaf_position);
        NodeCurrent3D *nc2 = new NodeCurrent3D(p2, limit_bisection, previmer, leaf_list, leaf_position);
        m_children_first = nc1;
        m_children_second = nc2;
    }
}

NodeCurrent3D::~NodeCurrent3D(){
    if(!is_leaf()){
        delete(m_children_first);
        delete(m_children_second);
    }
}

void NodeCurrent3D::compute_vector_field_tree(const ibex::IntervalVector &position, short &min_u, short &max_u,
                                              short &min_v, short &max_v){
    if(is_leaf()){
        union_vector(min_u, max_u, min_v, max_v);
    }
    else{
        IntervalVector p1(position);
        IntervalVector p2(position);
        p1[m_bisection_axis] = Interval(position[m_bisection_axis].lb(),position[m_bisection_axis].mid());
        p2[m_bisection_axis] = Interval(position[m_bisection_axis].mid(),position[m_bisection_axis].ub());

        m_children_first->compute_vector_field_tree(p1, min_u, max_u, min_v, max_v);
        m_children_second->compute_vector_field_tree(p2, min_u, max_u, min_v, max_v);
        m_min_u = min_u;
        m_min_v = min_v;
        m_max_u = max_u;
        m_max_v = max_v;
    }
}

void NodeCurrent3D::eval(const IntervalVector& target, const IntervalVector& position, short &min_u, short &max_u,
                                               short &min_v, short &max_v) const{
    if(is_leaf()){
        union_vector(min_u, max_u, min_v, max_v);
    }
    else{
        IntervalVector inter = target & position;
        if(inter.is_empty()){
            return;
        }
        else if(position.is_subset(target)){
            union_vector(min_u, max_u, min_v, max_v);
        }
        else{
            // Bisect
            IntervalVector p1(position);
            IntervalVector p2(position);
            p1[m_bisection_axis] = Interval(position[m_bisection_axis].lb(),position[m_bisection_axis].mid());
            p2[m_bisection_axis] = Interval(position[m_bisection_axis].mid(),position[m_bisection_axis].ub());
            m_children_first->eval(target, p1, min_u, max_u, min_v, max_v);
            m_children_second->eval(target, p2, min_u, max_u, min_v, max_v);
        }
    }
}

void NodeCurrent3D::union_vector(short &min_u, short &max_u,short &min_v, short &max_v) const{
    min_u = min(min_u, m_min_u);
    min_v = min(min_v, m_min_v);
    max_u = max(max_u, m_max_u);
    max_v = max(max_v, m_max_v);
}

}

