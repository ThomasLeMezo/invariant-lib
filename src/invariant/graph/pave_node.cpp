#include "pave_node.h"
#include <utility>
using namespace std;
using namespace ibex;

namespace invariant{
Pave_node::Pave_node(Pave *p):m_position(p->get_position())
{
    m_leaf = true;
    m_pave = p;
}

void Pave_node::add_child(Pave *p1, Pave *p2){
    m_leaf = false;
    m_pave = NULL;
    Pave_node* pn1 = new Pave_node(p1);
    Pave_node* pn2 = new Pave_node(p2);
    m_children = make_pair(pn1, pn2);

    p1->set_pave_node(pn1);
    p2->set_pave_node(pn2);
}

Pave_node::~Pave_node(){
    if(m_leaf==false){
        delete(m_children.first);
        delete(m_children.second);
    }
}

void Pave_node::get_intersection_pave_outer(std::vector<Pave*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf)
            l.push_back(m_pave);
        else{
            m_children.first->get_intersection_pave_outer(l, box);
            m_children.second->get_intersection_pave_outer(l, box);
        }
    }
}

void Pave_node::get_intersection_pave_inner(std::vector<Pave*> &l, const ibex::IntervalVector &box){
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

void Pave_node::get_intersection_pave_outer(std::vector<Pave*> &l, ibex::Ctc &nc){
    IntervalVector position(m_position);
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

void Pave_node::get_intersection_pave_inner(std::vector<Pave*> &l, ibex::Ctc &nc){
    IntervalVector position(m_position);
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

void Pave_node::get_intersection_face_outer(std::vector<Face*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            for(Face*f:m_pave->get_faces_vector()){
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

void Pave_node::get_intersection_face_inner(std::vector<Face*> &l, const ibex::IntervalVector &box){
    if(!(box & m_position).is_empty()){
        if(m_leaf){
            for(Face*f:m_pave->get_faces_vector()){
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

void Pave_node::get_intersection_face_outer(std::vector<Face*> &l, ibex::Ctc &nc){
    IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            for(Face*f:m_pave->get_faces_vector()){
                IntervalVector f_position(f->get_position());
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

void Pave_node::get_intersection_face_inner(std::vector<Face*> &l, ibex::Ctc &nc){
    IntervalVector position(m_position);
    nc.contract(position);

    if(!position.is_empty()){
        if(m_leaf){
            if(position == m_position){
                for(Face*f:m_pave->get_faces_vector()){
                    IntervalVector f_position(f->get_position());
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

}
