#include "pave_node.h" // only for ide

namespace invariant{
template<typename _Tp, typename _V>
Pave_node<_Tp, _V>::Pave_node(Pave<_Tp, _V> *p):m_position(p->get_position())
{
    m_leaf = true;
    m_pave = p;
    m_border_pave = p->is_border();
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::add_child(Pave<_Tp, _V> *p1, Pave<_Tp, _V> *p2){
    m_leaf = false;
    m_pave = NULL;
    Pave_node<_Tp, _V>* pn1 = new Pave_node<_Tp, _V>(p1);
    Pave_node<_Tp, _V>* pn2 = new Pave_node<_Tp, _V>(p2);
    m_children = std::make_pair(pn1, pn2);

    p1->set_pave_node(pn1);
    p2->set_pave_node(pn2);
}

template<typename _Tp, typename _V>
Pave_node<_Tp, _V>::~Pave_node(){
    if(m_leaf==false){
        delete(m_children.first);
        delete(m_children.second);
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_pave_outer(std::vector<Pave<_Tp, _V>*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf)
            l.push_back(m_pave);
        else{
            m_children.first->get_intersection_pave_outer(l, box);
            m_children.second->get_intersection_pave_outer(l, box);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_pave_inner(std::vector<Pave<_Tp, _V>*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            if(m_position.is_strict_interior_subset(box))
                l.push_back(m_pave);
        }
        else{
            m_children.first->get_intersection_pave_inner(l, box);
            m_children.second->get_intersection_pave_inner(l, box);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_pave_outer(std::vector<Pave<_Tp, _V>*> &l, ibex::Ctc &nc){
    ibex::IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            l.push_back(m_pave);
        }
        else{
            m_children.first->get_intersection_pave_outer(l, nc);
            m_children.second->get_intersection_pave_outer(l, nc);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_pave_inner(std::vector<Pave<_Tp, _V>*> &l, ibex::Ctc &nc){
    ibex::IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            if(position == m_position){
                l.push_back(m_pave);
            }
        }
        else{
            m_children.first->get_intersection_pave_inner(l, nc);
            m_children.second->get_intersection_pave_inner(l, nc);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_face_outer(std::vector<Face<_Tp, _V>*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            for(Face<_Tp, _V>*f:m_pave->get_faces_vector()){
                if(!(f->get_position() & box).is_empty())
                    l.push_back(f);
            }
        }
        else{
            m_children.first->get_intersection_face_outer(l, box);
            m_children.second->get_intersection_face_outer(l, box);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_face_inner(std::vector<Face<_Tp, _V>*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            for(Face<_Tp, _V>*f:m_pave->get_faces_vector()){
                if(f->get_position().is_strict_interior_subset(box))
                    l.push_back(f);
            }
        }
        else{
            m_children.first->get_intersection_face_inner(l, box);
            m_children.second->get_intersection_face_inner(l, box);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_face_outer(std::vector<Face<_Tp, _V>*> &l, ibex::Ctc &nc){
    ibex::IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            for(Face<_Tp, _V>*f:m_pave->get_faces_vector()){
                ibex::IntervalVector f_position(f->get_position());
                nc.contract(f_position);
                if(!f_position.is_empty())
                    l.push_back(f);
            }
        }
        else{
            m_children.first->get_intersection_face_outer(l, nc);
            m_children.second->get_intersection_face_outer(l, nc);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_face_inner(std::vector<Face<_Tp, _V>*> &l, ibex::Ctc &nc){
    ibex::IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            if(position == m_position){
                for(Face<_Tp, _V>*f:m_pave->get_faces_vector()){
                    ibex::IntervalVector f_position(f->get_position());
                    nc.contract(f_position);
                    if(f_position == f->get_position())
                        l.push_back(f);
                }
            }
        }
        else{
            m_children.first->get_intersection_face_inner(l, nc);
            m_children.second->get_intersection_face_inner(l, nc);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_all_child_rooms(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const{
    if(is_leaf()){
        list_room.push_back(m_pave->get_rooms()[maze]);
    }
    else{
        m_children.first->get_all_child_rooms(list_room, maze);
        m_children.second->get_all_child_rooms(list_room, maze);
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_all_child_rooms_not_empty(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const{
    if(is_leaf()){
        Room<_Tp, _V> *r = m_pave->get_rooms()[maze];
        if(!r->is_empty() && !r->is_removed())
            list_room.push_back(m_pave->get_rooms()[maze]);
    }
    //    else if(this->get_fullness()[maze]){ // Not right because of contraction of the yellow area
    //        m_children.first->get_all_child_rooms(list_room, maze);
    //        m_children.second->get_all_child_rooms(list_room, maze);
    //    }
    else{
        if(!this->get_removed()[maze] && !this->get_emptyness()[maze]){
            m_children.first->get_all_child_rooms_not_empty(list_room, maze);
            m_children.second->get_all_child_rooms_not_empty(list_room, maze);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_all_child_rooms_not_full(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const{
    if(is_leaf()){
        Room<_Tp, _V> *r = m_pave->get_rooms()[maze];
        if(!r->is_full() && !r->is_removed())
            list_room.push_back(m_pave->get_rooms()[maze]);
    }
    else if(this->get_emptyness()[maze]){
        m_children.first->get_all_child_rooms(list_room, maze);
        m_children.second->get_all_child_rooms(list_room, maze);
    }
    else{
        if(!this->get_fullness()[maze] && !this->get_removed()[maze]){
            m_children.first->get_all_child_rooms_not_full(list_room, maze);
            m_children.second->get_all_child_rooms_not_full(list_room, maze);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_all_child_rooms_inside_outside(std::vector<Room<_Tp, _V> *> &list_room, Maze<_Tp, _V> *maze) const{
    // ToDo : verify ? => why not called recursively ?
    if(is_leaf()){
        Room<_Tp, _V> *r = m_pave->get_rooms()[maze];
        if(!r->is_full() && !r->is_removed())
            list_room.push_back(m_pave->get_rooms()[maze]);
    }
    else{
        if(!this->get_removed()[maze]){
            m_children.first->get_all_child_rooms_not_full(list_room, maze);
            m_children.second->get_all_child_rooms_not_full(list_room, maze);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_border_paves(std::vector<Pave<_Tp, _V>*>& pave_list) const{
    if(is_leaf() && m_pave->is_border()){
        pave_list.push_back(m_pave);
    }
    else{
        if(m_border_pave){
            m_children.first->get_border_paves(pave_list);
            m_children.second->get_border_paves(pave_list);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_polygon_not_empty(std::vector<Room<_Tp, _V>*> &l, const ibex::IntervalVector &box, Maze<_Tp, _V> *maze) const{
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            Room<_Tp, _V> *r = m_pave->get_rooms()[maze];
            if(!r->is_removed() && !r->is_empty()){
                l.push_back(r);
            }
        }
        else{
            m_children.first->get_intersection_polygon_not_empty(l, box, maze);
            m_children.second->get_intersection_polygon_not_empty(l, box, maze);
        }
    }
}

template<typename _Tp, typename _V>
void Pave_node<_Tp, _V>::get_intersection_polygon_empty(std::vector<Room<_Tp, _V>*> &l, const ibex::IntervalVector &box, Maze<_Tp, _V> *maze) const{
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            Room<_Tp, _V> *r = m_pave->get_rooms()[maze];
            if(r->is_removed() || !r->is_full()){
                l.push_back(r);
            }
        }
        else{
            m_children.first->get_intersection_polygon_empty(l, box, maze);
            m_children.second->get_intersection_polygon_empty(l, box, maze);
        }
    }
}

}
