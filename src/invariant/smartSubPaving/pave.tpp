using namespace std;
using namespace ibex;

#include "pave.h"

namespace invariant {

template<typename _Tp>
Pave<_Tp>::Pave(const ibex::IntervalVector &position, SmartSubPaving<_Tp> *g):
    m_position(position)
{
    m_subpaving = g;
    m_serialization_id = 0;
    m_dim = g->dim();

    if(position.is_unbounded())
        m_infinite_pave = true;
    ibex::IntervalVector normal(m_dim, ibex::Interval(0));

    // Build the faces
    for(size_t i=0; i<m_dim; i++){
        ibex::IntervalVector normal_lb(normal);
        ibex::IntervalVector normal_ub(normal);
        normal_lb[i] = ibex::Interval(1);
        normal_ub[i] = ibex::Interval(-1);

        ibex::IntervalVector iv_lb(m_position);
        ibex::IntervalVector iv_ub(m_position);
        ibex::IntervalVector orient_lb(m_dim, ibex::Interval(0, 1));
        ibex::IntervalVector orient_ub(m_dim, ibex::Interval(0, 1));

        iv_lb[i]=ibex::Interval(m_position[i].lb());
        iv_ub[i]=ibex::Interval(m_position[i].ub());
        orient_lb[i] = ibex::Interval(0);
        orient_ub[i] = ibex::Interval(1);

        Face<_Tp>* face_lb = new Face<_Tp>(iv_lb, orient_lb, normal_lb, this);
        Face<_Tp>* face_ub = new Face<_Tp>(iv_ub, orient_ub, normal_ub, this);

        std::array<Face<_Tp>*, 2> face_array = {face_lb, face_ub};
        m_faces.push_back(face_array);
        m_faces_vector.push_back(face_lb);
        m_faces_vector.push_back(face_ub);
    }
    compute_typed_position();
}

template<typename _Tp>
Pave<_Tp>::Pave(SmartSubPaving<_Tp> *g):
    m_position(0)
{
    m_subpaving = g;
}

template<typename _Tp>
Pave<_Tp>::~Pave(){
    for(typename std::map<Maze<_Tp>*,Room<_Tp>*>::iterator it=m_rooms.begin(); it!=m_rooms.end(); ++it){
        delete(it->second);
    }
    for(Face<_Tp>* f:m_faces_vector){
        delete(f);
    }
    delete(m_position_typed);
}

template<typename _Tp>
void Pave<_Tp>::serialize(std::ofstream& binFile) const{
    // *** Pave serialization ***
    // size_t           Serialization id
    // ibex::IntervalVector   m_position
    // [...] Faces

    /// ToDo : add pave node tree to the serialization process + update others variables

    binFile.write((const char*) &m_serialization_id, sizeof(size_t)); // Serialization id
    serializeIntervalVector(binFile, m_position);

    // Faces serialization
    for(size_t i=0; i<m_dim; i++){
        for(unsigned char j=0; j<2; j++){
            m_faces[i][j]->serialize(binFile);
        }
    }
}

template<typename _Tp>
void Pave<_Tp>::deserialize(std::ifstream& binFile){
    binFile.read((char*)&m_serialization_id, sizeof(size_t));
    m_position = deserializeIntervalVector(binFile);

    /// ToDo : add pave node tree to the deserialization process + update others variables

    // Create Faces
    const unsigned char dim = m_dim;
    for(unsigned char i=0; i<dim; i++){
        Face<_Tp>* face_lb = new Face<_Tp>(this);
        Face<_Tp>* face_ub = new Face<_Tp>(this);
        std::array<Face<_Tp>*, 2> face_array = {face_lb, face_ub};
        m_faces.push_back(face_array);
    }

    // Deserialize Faces & restore pointers link
    for(unsigned char i=0; i<dim; i++){
        m_faces[i][0]->deserialize(binFile);
        m_faces[i][1]->deserialize(binFile);
    }
}

template<typename _Tp>
std::ostream& operator<< (std::ostream& stream, const std::vector<Pave<_Tp>*> &l){
    for(Pave<_Tp> *p:l){
        stream << *p << std::endl;
    }
    return stream;
}

template<typename _Tp>
const bool Pave<_Tp>::is_equal(const Pave<_Tp>& p) const{
    if(m_position != p.get_position())
        return false;
    for(size_t i=0; i<m_dim; i++){
        for(size_t j=0; j<2; j++){
            if(!(m_faces[i][j]->is_equal(*(p[i][j]))))
                return false;
        }
    }
    return true;
}

template<typename _Tp>
void Pave<_Tp>::bisect(){
    //    ibex::LargestFirst bisector(0, 0.5);
    //    std::pair<ibex::IntervalVector> result_boxes = bisector.bisect(m_position);

    std::pair<ibex::IntervalVector, ibex::IntervalVector> result_boxes = m_subpaving->bisect_largest_first(m_position);
    const size_t dim = m_dim;
    // Find the axe of bissection
    size_t bisect_axis = 0;
    for(size_t i=0; i<dim; i++){
        if(result_boxes.first[i] != m_position[i]){
            bisect_axis = (size_t)i;
            break;
        }
    }

    // Create new Paves
    Pave<_Tp> *pave0 = new Pave<_Tp>(result_boxes.first, m_subpaving); // lb
    Pave<_Tp> *pave1 = new Pave<_Tp>(result_boxes.second, m_subpaving); // ub
    std::array<Pave<_Tp>*, 2> pave_result = {pave0, pave1};

    // 1) Update paves neighbors with the new two paves
    for(size_t face=0; face<dim; face++){
        for(int sens=0; sens<2; sens++){
            for(Face<_Tp> *f:m_faces[face][sens]->get_neighbors()){
                f->remove_neighbor(m_faces[face][sens]);

                if(face==bisect_axis){
                    f->add_neighbor(pave_result[sens]->get_faces()[face][sens]);
                }
                else{
                    f->add_neighbor(pave_result[0]->get_faces()[face][sens]);
                    f->add_neighbor(pave_result[1]->get_faces()[face][sens]);
                }
            }
        }
    }

    // 2) Copy brothers Pave (this) to pave1 and pave2
    for(size_t face=0; face<dim; face++){
        for(size_t sens=0; sens<2; sens++){
            for(Face<_Tp> *f:m_faces[face][sens]->get_neighbors()){
                if(!((face==bisect_axis) & (sens==1)))
                    pave_result[0]->get_faces()[face][sens]->add_neighbor(f);
                if(!((face==bisect_axis) & (sens==0)))
                    pave_result[1]->get_faces()[face][sens]->add_neighbor(f);
            }
        }
    }

    // 3) Add inter link
    pave_result[1]->get_faces()[bisect_axis][0]->add_neighbor(pave_result[0]->get_faces()[bisect_axis][1]);
    pave_result[0]->get_faces()[bisect_axis][1]->add_neighbor(pave_result[1]->get_faces()[bisect_axis][0]);

    // 4) Add Paves to the paving
    m_subpaving->add_paves(pave_result[0]);
    m_subpaving->add_paves(pave_result[1]);

    // 5) Analyze border
    if(this->is_border()){
        pave_result[0]->analyze_border();
        pave_result[1]->analyze_border();
    }

    // 6) Add new node to the tree
    m_tree->add_child(pave_result[0], pave_result[1]);

    // Add Room to the Paves
    for(typename std::map<Maze<_Tp>*,Room<_Tp>*>::iterator it=m_rooms.begin(); it!=m_rooms.end(); ++it){
        Room<_Tp> *r = (it->second);

        Room<_Tp> *r_first = new Room<_Tp>(pave_result[0],r->get_maze(), r->get_maze()->get_dynamics());
        Room<_Tp> *r_second = new Room<_Tp>(pave_result[1],r->get_maze(), r->get_maze()->get_dynamics());

        // ToDo : add a contraction of Room(s) according to father

        pave_result[0]->add_room(r_first);
        pave_result[1]->add_room(r_second);

        if(r->is_empty()){
            m_tree->add_emptyness((it->first), true);
        }
        else
            m_tree->add_emptyness((it->first), false);

        if(r->is_full()){
            m_tree->add_fullness((it->first), true);
            // Child cannot be set to full because of overapproximation
            // in the case of FULL_WALL & FULL_DOOR is already full
        }
        else
            m_tree->add_fullness((it->first), false);

        if(r->is_removed()){
            m_tree->add_removed((it->first), true);
            if(r->is_full()){
                r_first->set_empty_private();
                r_second->set_full_private();
            }
            if(r->is_empty()){
                r_first->set_empty_private();
                r_second->set_empty_private();
            }
            r_first->synchronize();
            r_first->set_removed();
            r_second->synchronize();
            r_second->set_removed();
        }
        else{
            m_tree->add_removed((it->first), false);
            r_first->synchronize();
            r_second->synchronize();
        }
    }

//    // Save results in this pave
//    m_result_bisected[0] = pave_result[0];
//    m_result_bisected[1] = pave_result[1];
}

template<typename _Tp>
const bool Pave<_Tp>::request_bisection(){
    bool request_wall = false;
    bool one_wall = false;

    bool request_door = false;
    bool one_door = false;

    for(typename std::map<Maze<_Tp>*,Room<_Tp>*>::iterator it=m_rooms.begin(); it!=m_rooms.end(); ++it){
        Room<_Tp> *r = it->second;
        Maze<_Tp> *maze = it->first;
        if(maze->get_domain()->get_init()==FULL_WALL){
            request_wall |= r->request_bisection();
            one_wall = true;
        }
        else{
            request_door |= r->request_bisection();
            one_door = true;
        }
    }

    return (!one_wall | request_wall) & (!one_door | request_door);
}

template<typename _Tp>
void Pave<_Tp>::set_removed_rooms(){
    for(typename std::map<Maze<_Tp>*,Room<_Tp>*>::iterator it=m_rooms.begin(); it!=m_rooms.end(); ++it){
        Room<_Tp>* r = it->second;
        //        if(r->is_empty()) //?
        if(r->is_empty() || r->get_maze()->get_domain()->get_init()!=FULL_WALL){
            r->set_removed();
            m_tree->set_removed(it->first);
        }
    }
}

template<typename _Tp>
void Pave<_Tp>::add_room(Room<_Tp> *r){
    m_rooms.insert(std::pair<Maze<_Tp>*,Room<_Tp>*>(r->get_maze(),r));
}

template<typename _Tp>
void Pave<_Tp>::analyze_border(){
    for(Face<_Tp>*f:m_faces_vector)
        m_border |= f->analyze_border();
}

template<typename _Tp>
void Pave<_Tp>::get_neighbors_pave(std::vector<Pave<_Tp>*> pave_list){
    for(Face<_Tp> *f:m_faces_vector){
        for(Face<_Tp> *f_n:f->get_neighbors()){
            pave_list.push_back(f_n->get_pave());
        }
    }
}

template<typename _Tp>
void Pave<_Tp>::get_neighbors_room(Maze<_Tp> *maze, std::vector<Room<_Tp>*>& room_list){
    for(Face<_Tp> *f:m_faces_vector){
        for(Face<_Tp> *f_n:f->get_neighbors()){
            Room<_Tp> *r_n = f_n->get_pave()->get_rooms()[maze];
            if(!r_n->is_removed())
                room_list.push_back(r_n);
        }
    }
}

template<typename _Tp>
int Pave<_Tp>::get_dim_inter_boundary(const ibex::IntervalVector &box){
    return 0;
}

}
