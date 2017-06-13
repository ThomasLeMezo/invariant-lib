#include "pave.h"
#include <serialization.h>

using namespace std;
using namespace ibex;

namespace invariant {

Pave::Pave(const ibex::IntervalVector &coordinates, Graph *g):
    m_coordinates(coordinates)
{
    m_graph = g;
    m_serialization_id = 0;
    unsigned char dim = g->dim();
    // Build the faces
    for(size_t i=0; i<dim; i++){
        IntervalVector iv_lb(m_coordinates);
        IntervalVector iv_ub(m_coordinates);
        IntervalVector orient_lb(dim, Interval(0, 1));
        IntervalVector orient_ub(dim, Interval(0,1));

        iv_lb[i]=Interval(m_coordinates[i].lb());
        iv_ub[i]=Interval(m_coordinates[i].ub());
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
    m_coordinates(0)
{
    m_graph = g;
}

const ibex::IntervalVector& Pave::coordinates() const
{
    return m_coordinates;
}

std::vector<std::array<Face *, 2>> Pave::faces() const
{
    return m_faces;
}

size_t Pave::getSerialization_id() const
{
    return m_serialization_id;
}

void Pave::setSerialization_id(size_t &value)
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
    // IntervalVector   m_coordinates
    // [...] Faces

    binFile.write((const char*) &m_serialization_id, sizeof(size_t)); // Serialization id
    ibex_tools::serializeIntervalVector(binFile, m_coordinates);

    // Faces serialization
    for(size_t i=0; i<m_faces.size(); i++){
        for(unsigned char j=0; j<2; j++){
            m_faces[i][j]->serialize(binFile);
        }
    }
}

void Pave::deserialize(std::ifstream& binFile){
    binFile.read((char*)&m_serialization_id, sizeof(size_t));
    m_coordinates = ibex_tools::deserializeIntervalVector(binFile);

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
    stream << p.coordinates();
    return stream;
}

bool Pave::operator==(const Pave& p) const{
    if(m_coordinates != p.coordinates())
        return false;
    for(size_t i=0; i<m_faces.size(); i++){
        for(size_t j=0; j<m_faces[i].size(); j++){
            if(*(m_faces[i][j]) != *(p[i][j]))
                return false;
        }
    }
    return true;
}

bool Pave::operator!=(const Pave& p) const{
    return !(*this == p);
}

const std::array<Face*, 2>& Pave::operator[](std::size_t i) const{
    return m_faces[i];
}

void Pave::bisect(){
    ibex::LargestFirst bisector(0, 0.5);
    std::pair<IntervalVector, IntervalVector> result_boxes = bisector.bisect(m_coordinates);

    // Find the axe of bissection
    size_t bisect_axis = 0;
    for(int i=0; i<m_coordinates.size(); i++){
        if(result_boxes.first[i] != m_coordinates[i]){
            bisect_axis = (size_t)i;
            break;
        }
    }

    // Create new Paves
    Pave *pave0 = new Pave(result_boxes.first, m_graph); // lb
    Pave *pave1 = new Pave(result_boxes.second, m_graph); // ub
    std::array<Pave*, 2> pave_result = {pave0, pave1};

    // 1) Update paves neighbors with the new two paves
    for(size_t face=0; face<m_faces.size(); face++){
        for(int sens=0; sens<1; sens++){
            for(Face *f:m_faces[face][sens]->neighbors()){
                if(face==bisect_axis){
                    if(f->coordinates().size()==0)
                        cout << "error" << endl;

                    Face *f0 = pave_result[sens]->faces()[face][sens];
                    cout << f0->coordinates() << '\t' ;
                    cout << f->coordinates() << endl;
                    f->add_neighbor(f0);
                }
                else{
                    Face *f1 = pave_result[0]->faces()[face][sens];
                    Face *f2 = pave_result[1]->faces()[face][sens];
                    f->add_neighbor(f1);
                    f->add_neighbor(f2);
                }
                f->remove_neighbor(m_faces[face][sens]);
            }
        }
    }

    // 2) Copy brothers Pave (this) to pave1 and pave2
    for(size_t face=0; face<m_faces.size(); face++){
        for(size_t sens=0; sens<1; sens++){
            for(Face *f:m_faces[face][sens]->neighbors()){
                if(!(face==bisect_axis && sens==1))
                    pave_result[0]->faces()[face][sens]->add_neighbor(f);
                if(!(face==bisect_axis && sens==0))
                    pave_result[1]->faces()[face][sens]->add_neighbor(f);
            }
        }
    }

    // 3) Add inter link
    pave_result[0]->faces()[bisect_axis][1]->add_neighbor(pave_result[1]->faces()[bisect_axis][0]);
    pave_result[1]->faces()[bisect_axis][0]->add_neighbor(pave_result[0]->faces()[bisect_axis][1]);

    // Add Paves to the graph
    m_graph->add_paves(pave_result[0]);
    m_graph->add_paves(pave_result[1]);

    // Save results in this pave
    m_result_bisected[0] = pave_result[0];
    m_result_bisected[1] = pave_result[1];
}

bool Pave::request_bisection(){
    return true;
}

std::array<Pave *, 2>& Pave::getResult_bisected()
{
    return m_result_bisected;
}

std::vector<Face *>& Pave::getFaces_vector()
{
    return m_faces_vector;
}

}
