#include "pave_node.h" // only for ide

namespace invariant{
template<typename _Tp>
Pave_node<_Tp>::Pave_node(Pave<_Tp> *p):m_position(p->get_position())
{
    m_leaf = true;
    m_pave = p;
    m_border_pave = p->is_border();
}

template<typename _Tp>
void Pave_node<_Tp>::add_child(Pave<_Tp> *p1, Pave<_Tp> *p2){
    m_leaf = false;
    m_pave = NULL;
    Pave_node<_Tp>* pn1 = new Pave_node<_Tp>(p1);
    Pave_node<_Tp>* pn2 = new Pave_node<_Tp>(p2);
    m_children = std::make_pair(pn1, pn2);

    p1->set_pave_node(pn1);
    p2->set_pave_node(pn2);
}

template<typename _Tp>
Pave_node<_Tp>::~Pave_node(){
    if(m_leaf==false){
        delete(m_children.first);
        delete(m_children.second);
    }
}

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_pave_outer(std::vector<Pave<_Tp>*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf)
            l.push_back(m_pave);
        else{
            m_children.first->get_intersection_pave_outer(l, box);
            m_children.second->get_intersection_pave_outer(l, box);
        }
    }
}

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_pave_inner(std::vector<Pave<_Tp>*> &l, const ibex::IntervalVector &box){
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_pave_outer(std::vector<Pave<_Tp>*> &l, ibex::Ctc &nc){
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_pave_inner(std::vector<Pave<_Tp>*> &l, ibex::Ctc &nc){
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_face_outer(std::vector<Face<_Tp>*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            for(Face<_Tp>*f:m_pave->get_faces_vector()){
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_face_inner(std::vector<Face<_Tp>*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            for(Face<_Tp>*f:m_pave->get_faces_vector()){
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_face_outer(std::vector<Face<_Tp>*> &l, ibex::Ctc &nc){
    ibex::IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            for(Face<_Tp>*f:m_pave->get_faces_vector()){
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_face_inner(std::vector<Face<_Tp>*> &l, ibex::Ctc &nc){
    ibex::IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            if(position == m_position){
                for(Face<_Tp>*f:m_pave->get_faces_vector()){
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

template<typename _Tp>
void Pave_node<_Tp>::get_all_child_rooms(std::vector<Room<_Tp> *> &list_room, Maze<_Tp> *maze) const{
    if(is_leaf()){
        list_room.push_back(m_pave->get_rooms()[maze]);
    }
    else{
        m_children.first->get_all_child_rooms(list_room, maze);
        m_children.second->get_all_child_rooms(list_room, maze);
    }
}

template<typename _Tp>
void Pave_node<_Tp>::get_all_child_rooms_not_empty(std::vector<Room<_Tp> *> &list_room, Maze<_Tp> *maze) const{
    if(is_leaf()){
        Room<_Tp> *r = m_pave->get_rooms()[maze];
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

template<typename _Tp>
void Pave_node<_Tp>::get_all_child_rooms_not_full(std::vector<Room<_Tp> *> &list_room, Maze<_Tp> *maze) const{
    if(is_leaf()){
        Room<_Tp> *r = m_pave->get_rooms()[maze];
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

template<typename _Tp>
void Pave_node<_Tp>::get_all_child_rooms_inside_outside(std::vector<Room<_Tp> *> &list_room, Maze<_Tp> *maze) const{
    // ToDo : verify ? => why not called recursively ?
    if(is_leaf()){
        Room<_Tp> *r = m_pave->get_rooms()[maze];
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

template<typename _Tp>
void Pave_node<_Tp>::get_bounding_fullness(Maze<_Tp> *maze, ibex::IntervalVector &result){
    if(m_position.is_subset(result))
        return;

    if(is_leaf()){
        Room<_Tp> *r = m_pave->get_rooms()[maze];
        if(!r->is_empty())
            result |= r->get_hull();
    }
    else{
        m_children.first->get_bounding_fullness(maze, result);
        m_children.second->get_bounding_fullness(maze, result);
    }
}

template<typename _Tp>
void Pave_node<_Tp>::get_border_paves(std::vector<Pave<_Tp>*>& pave_list) const{
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_polygon_not_empty(std::vector<Room<_Tp>*> &l, const ibex::IntervalVector &box, Maze<_Tp> *maze) const{
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            Room<_Tp> *r = m_pave->get_rooms()[maze];
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

template<typename _Tp>
void Pave_node<_Tp>::get_intersection_polygon_empty(std::vector<Room<_Tp>*> &l, const ibex::IntervalVector &box, Maze<_Tp> *maze) const{
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            Room<_Tp> *r = m_pave->get_rooms()[maze];
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
