#include "face.h"

using namespace std;

namespace invariant {

template <typename _Tp>
Face<_Tp>::Face(const ibex::IntervalVector &position, const ibex::IntervalVector &orientation, const ibex::IntervalVector &normal, Pave<_Tp> *p):
    m_position(position),
    m_orientation(orientation),
    m_normal(normal)
{
    m_pave = p;
}

template <typename _Tp>
Face<_Tp>::~Face(){
    for(typename std::map<Maze<_Tp>*,Door<_Tp>*>::iterator it=m_doors.begin(); it!=m_doors.end(); ++it){
        delete(it->second);
    }
}

template <typename _Tp>
Face<_Tp>::Face(Pave<_Tp> *p):
    m_position(0),
    m_orientation(0),
    m_normal(0)
{
    m_pave = p;
}

template <typename _Tp>
void Face<_Tp>::add_neighbor(Face<_Tp> *f){
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

template <typename _Tp>
void Face<_Tp>::remove_neighbor(const Face<_Tp> *f){
    const size_t nb_neighbor = m_neighbors.size();
    for(size_t i=0; i<nb_neighbor; i++){
        if(m_neighbors[i] == f){ // pointer test
            m_neighbors.erase(m_neighbors.begin()+i);
            return;
        }
    }
    throw std::runtime_error("in [face.cpp/remove_neighobr] neighbor face was not found which is not expected");
}

template <typename _Tp>
std::ostream& operator<< (std::ostream& stream, const std::vector<Face<_Tp>*> &l){
    for(Face<_Tp> *f:l){
        stream << *f << endl;
    }
    return stream;
}

template <typename _Tp>
void Face<_Tp>::add_door(Door<_Tp> *door){
    m_doors.insert(std::pair<Maze<_Tp>*,Door<_Tp>*>(door->get_room()->get_maze(), door));
}

}
