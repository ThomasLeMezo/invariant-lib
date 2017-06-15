#include "pave.h"
#include <serialization.h>

using namespace std;
using namespace ibex;

namespace invariant {

Pave::Pave(const ibex::IntervalVector &position, Graph *g):
    m_position(position)
{
    m_graph = g;
    m_serialization_id = 0;
    unsigned char dim = g->dim();
    // Build the faces
    for(size_t i=0; i<dim; i++){
        IntervalVector iv_lb(m_position);
        IntervalVector iv_ub(m_position);
        IntervalVector orient_lb(dim, Interval(0, 1));
        IntervalVector orient_ub(dim, Interval(0,1));

        iv_lb[i]=Interval(m_position[i].lb());
        iv_ub[i]=Interval(m_position[i].ub());
        orient_lb[i] = Interval(0);
        orient_ub[i] = Interval(1);

        Face* face_lb = new Face(iv_lb, orient_lb, this);
        Face* face_ub = new Face(iv_ub, orient_ub, this);

        array<Face*, 2> face_array = {face_lb, face_ub};
        m_faces.push_back(face_array);
        m_faces_vector.push_back(face_lb);
        m_faces_vector.push_back(face_ub);
    }
}

Pave::Pave(Graph *g):
    m_position(0)
{
    m_graph = g;
}

const ibex::IntervalVector& Pave::get_position() const
{
    return m_position;
}

const std::vector<std::array<Face *, 2> > &Pave::get_faces() const
{
    return m_faces;
}

const size_t& Pave::get_serialization_id() const
{
    return m_serialization_id;
}

void Pave::set_serialization_id(const size_t &value)
{
    m_serialization_id = value;
}

Pave::~Pave(){
    for(std::array<Face *, 2> &a:m_faces){
        for(Face* f:a){
            delete(f);
        }
    }
}

void Pave::serialize(std::ofstream& binFile) const{
    // *** Pave serialization ***
    // size_t           Serialization id
    // IntervalVector   m_position
    // [...] Faces

    /// ToDo : add pave node tree to the serialization process + update others variables

    binFile.write((const char*) &m_serialization_id, sizeof(size_t)); // Serialization id
    ibex_tools::serializeIntervalVector(binFile, m_position);

    // Faces serialization
    for(size_t i=0; i<m_faces.size(); i++){
        for(unsigned char j=0; j<2; j++){
            m_faces[i][j]->serialize(binFile);
        }
    }
}

void Pave::deserialize(std::ifstream& binFile){
    binFile.read((char*)&m_serialization_id, sizeof(size_t));
    m_position = ibex_tools::deserializeIntervalVector(binFile);

    /// ToDo : add pave node tree to the deserialization process + update others variables

    // Create Faces
    for(unsigned char i=0; i<m_graph->dim(); i++){
        Face* face_lb = new Face(this);
        Face* face_ub = new Face(this);
        array<Face*, 2> face_array = {face_lb, face_ub};
        m_faces.push_back(face_array);
    }

    // Deserialize Faces & restore pointers link
    for(unsigned char i=0; i<m_graph->dim(); i++){
        m_faces[i][0]->deserialize(binFile);
        m_faces[i][1]->deserialize(binFile);
    }
}

std::ostream& operator<< (std::ostream& stream, const Pave& p) {
    stream << p.get_position();
    return stream;
}

std::ostream& operator<< (std::ostream& stream, const std::vector<Pave*> &l){
    for(Pave *p:l){
        stream << *p << endl;
    }
    return stream;
}

const bool Pave::is_equal(const Pave& p) const{
    if(m_position != p.get_position())
        return false;
    for(size_t i=0; i<m_faces.size(); i++){
        for(size_t j=0; j<m_faces[i].size(); j++){
            if(!(m_faces[i][j]->is_equal(*(p[i][j]))))
                return false;
        }
    }
    return true;
}

const std::array<Face*, 2>& Pave::operator[](const std::size_t& i) const{
    return m_faces[i];
}

void Pave::bisect(){
    ibex::LargestFirst bisector(0, 0.5);
    std::pair<IntervalVector, IntervalVector> result_boxes = bisector.bisect(m_position);

    // Find the axe of bissection
    size_t bisect_axis = 0;
    for(int i=0; i<m_position.size(); i++){
        if(result_boxes.first[i] != m_position[i]){
            bisect_axis = (size_t)i;
            break;
        }
    }

    // Create new Paves
    Pave *pave0 = new Pave(result_boxes.first, m_graph); // lb
    Pave *pave1 = new Pave(result_boxes.second, m_graph); // ub
    std::array<Pave*, 2> pave_result = {pave0, pave1};
    m_pave_node->add_child(pave0, pave1);

    // 1) Update paves neighbors with the new two paves
    for(size_t face=0; face<m_faces.size(); face++){
        for(int sens=0; sens<2; sens++){
            for(Face *f:m_faces[face][sens]->get_neighbors()){
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
    for(size_t face=0; face<m_faces.size(); face++){
        for(size_t sens=0; sens<2; sens++){
            for(Face *f:m_faces[face][sens]->get_neighbors()){
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

    // Add Paves to the graph
    m_graph->add_paves(pave_result[0]);
    m_graph->add_paves(pave_result[1]);

    // Save results in this pave
    m_result_bisected[0] = pave_result[0];
    m_result_bisected[1] = pave_result[1];
}

const bool Pave::request_bisection(){
    return true;
}

const std::array<Pave *, 2>& Pave::get_result_bisected()
{
    return m_result_bisected;
}

const std::vector<Face *> &Pave::get_faces_vector()
{
    return m_faces_vector;
}

void Pave::set_pave_node(Pave_node *pave_node)
{
    m_pave_node = pave_node;
}

}
