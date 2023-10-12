#include "face.h"

namespace invariant {

//template <typename _TpR, typename _TpF, typename _TpD>
//Face<_TpR,_TpF,_TpD>::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, Pave<_TpR,_TpF,_TpD> *p):
//    m_position(position),
//    m_orientation(orientation),
//    m_normal(normal)
//{
//    m_pave = p;
//}

template <typename _TpR, typename _TpF, typename _TpD>
Face<_TpR,_TpF,_TpD>::~Face(){
    for(typename std::map<Maze<_TpR,_TpF,_TpD>*,Door<_TpR,_TpF,_TpD>*>::iterator it=m_doors.begin(); it!=m_doors.end(); ++it){
        delete(it->second);
    }
    omp_destroy_lock(&m_write_neighbors);
}

template <typename _TpR, typename _TpF, typename _TpD>
void Face<_TpR,_TpF,_TpD>::add_neighbor(Face<_TpR,_TpF,_TpD> *f){
    const ibex::IntervalVector r = m_position & f->get_position();
    int nb_not_flat = 0;
    for(int i=0; i<r.size(); i++){
        if(r[i].is_degenerated()){
            nb_not_flat++;
            if(nb_not_flat>1)
                return;
        }
    }
    omp_set_lock(&m_write_neighbors);
    m_neighbors.push_back(f);
    omp_unset_lock(&m_write_neighbors);
}

template <typename _TpR, typename _TpF, typename _TpD>
void Face<_TpR,_TpF,_TpD>::remove_neighbor(const Face<_TpR,_TpF,_TpD> *f){
    omp_set_lock(&m_write_neighbors);
    const size_t nb_neighbor = m_neighbors.size();
    for(size_t i=0; i<nb_neighbor; i++){
        if(m_neighbors[i] == f){ // pointer test
            m_neighbors.erase(m_neighbors.begin()+i);
            omp_unset_lock(&m_write_neighbors);
            return;
        }
    }
    omp_unset_lock(&m_write_neighbors);
    throw std::runtime_error("in [face.cpp/remove_neighobr] neighbor face was not found which is not expected");
}

template <typename _TpR, typename _TpF, typename _TpD>
std::ostream& operator<< (std::ostream& stream, const std::vector<Face<_TpR,_TpF,_TpD>*> &l){
    for(Face<_TpR,_TpF,_TpD> *f:l){
        stream << *f << std::endl;
    }
    return stream;
}

template <typename _TpR, typename _TpF, typename _TpD>
void Face<_TpR,_TpF,_TpD>::add_door(Door<_TpR,_TpF,_TpD> *door){
    m_doors.insert(std::pair<Maze<_TpR,_TpF,_TpD>*,Door<_TpR,_TpF,_TpD>*>(door->get_room()->get_maze(), door));
}

}
