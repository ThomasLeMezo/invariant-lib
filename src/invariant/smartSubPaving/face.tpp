#include "face.h"

namespace invariant {

//template <typename _Tp, typename _V>
//Face<_Tp, _V>::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, Pave<_Tp, _V> *p):
//    m_position(position),
//    m_orientation(orientation),
//    m_normal(normal)
//{
//    m_pave = p;
//}

template <typename _Tp, typename _V>
Face<_Tp, _V>::~Face(){
    for(typename std::map<Maze<_Tp, _V>*,Door<_Tp, _V>*>::iterator it=m_doors.begin(); it!=m_doors.end(); ++it){
        delete(it->second);
    }
}

template <typename _Tp, typename _V>
void Face<_Tp, _V>::add_neighbor(Face<_Tp, _V> *f){
    const ibex::IntervalVector r = m_position & f->get_position();
    int nb_not_flat = 0;
    for(int i=0; i<r.size(); i++){
        if(r[i].is_degenerated()){
            nb_not_flat++;
            if(nb_not_flat>1)
                return;
        }
    }
    m_neighbors.push_back(f);
}

template <typename _Tp, typename _V>
void Face<_Tp, _V>::remove_neighbor(const Face<_Tp, _V> *f){
    const size_t nb_neighbor = m_neighbors.size();
    for(size_t i=0; i<nb_neighbor; i++){
        if(m_neighbors[i] == f){ // pointer test
            m_neighbors.erase(m_neighbors.begin()+i);
            return;
        }
    }
    throw std::runtime_error("in [face.cpp/remove_neighobr] neighbor face was not found which is not expected");
}

template <typename _Tp, typename _V>
std::ostream& operator<< (std::ostream& stream, const std::vector<Face<_Tp, _V>*> &l){
    for(Face<_Tp, _V> *f:l){
        stream << *f << std::endl;
    }
    return stream;
}

template <typename _Tp, typename _V>
void Face<_Tp, _V>::add_door(Door<_Tp, _V> *door){
    m_doors.insert(std::pair<Maze<_Tp, _V>*,Door<_Tp, _V>*>(door->get_room()->get_maze(), door));
}

}
