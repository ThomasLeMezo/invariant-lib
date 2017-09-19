#include "graph.h"
#include "serialization.h"
#include <stdexcept>
#include <fstream>

using namespace std;
using namespace ibex;

namespace invariant {
Graph::Graph(const ibex::IntervalVector &space):
    m_position(space)
{
    m_dim = (unsigned char) space.size();

    // Create search space Pave
    Pave* p = new Pave(space, this);
    m_paves.push_back(p);

    // Create infinity Paves around search space
//    IntervalVector* result;
//    int n=space.complementary(result);

//    for (int i=0; i<n; i++) {
//        Pave* p_infinity = new Pave(result[i], this);
//        cout << result[i] << endl;
//        m_paves_not_bisectable.push_back(p_infinity);
//    }

    // Analyze faces (border)
    for(Pave *p:m_paves)
        p->analyze_border();
    for(Pave *p:m_paves_not_bisectable)
        p->analyze_border();

    // Root of the pave node tree
    m_tree = new Pave_node(p);
    p->set_pave_node(m_tree);

    // Compute ratio dimension
    for(int dim=0; dim<m_dim; dim++){
        double diam = space[dim].diam();
        m_ratio_dimension.push_back(1.0/diam);
        m_limit_bisection.push_back(0.0);
    }
}

Graph::~Graph(){
    for(Pave *p:m_paves){
        if(p!=NULL)
            delete(p);
    }
    delete(m_tree);
}

void Graph::delete_pave(int id){
    delete(m_paves[id]);
    m_paves[id] = NULL;
}

void Graph::serialize(std::ofstream& binFile) const{
    // unsigned char    dimension
    // size_t           number of paves
    // IntervalVector   position
    // [...] Paves of the graph

    binFile.write((const char*)&m_dim, sizeof(unsigned char)); // dimension
    size_t size = m_paves.size();
    binFile.write((const char*)&size, sizeof(size_t)); // Number of paves
    ibex_tools::serializeIntervalVector(binFile, m_position);

    size_t cpt = 0;
    for(Pave *p:m_paves){
        p->set_serialization_id(cpt); cpt++;
        p->serialize(binFile);
    }
}

void Graph::deserialize(std::ifstream& binFile){
    if(m_paves.size()!=0){
        throw std::runtime_error("in [graph.cpp/deserialize] Graph is not empty");
        return;
    }

    binFile.read((char*)&m_dim, sizeof(unsigned char));
    size_t number_pave;
    binFile.read((char*)&number_pave, sizeof(size_t));
    m_position = ibex_tools::deserializeIntervalVector(binFile);

    const size_t number_pave_const = number_pave;
    for(size_t i=0; i<number_pave_const; i++){
        Pave *p = new Pave(this);
        m_paves.push_back(p);
    }
    for(size_t i=0; i<number_pave_const; i++){
        Pave *p = m_paves[i];
        p->deserialize(binFile);
    }
}

const bool Graph::is_equal(const Graph& g) const{
    if(m_position != g.get_position())
        return false;
    if(m_dim != g.dim())
        return false;
    const size_t nb_pave = m_paves.size();
    for(size_t i=0; i<nb_pave; i++){
        if(!(m_paves[i]->is_equal(*(g[i]))))
            return false;
    }
    return true;
}

void Graph::bisect(){
    vector<Pave*> m_bisectable_paves = m_paves;
    vector<Pave*> m_bisected_paves;
    m_paves.clear();

    /// Bisect the graph ///
    while(m_bisectable_paves.size()>0){
        Pave *p = m_bisectable_paves.back();
        m_bisectable_paves.pop_back();

        if(p->request_bisection()){
            p->bisect(); // bisected added to m_paves & update mazes
            delete(p);
        }
        else{
            // I
//            for(Maze *maze:m_mazes){
//                if(maze->get_type() == MAZE_WALL){
//                    Room *r = p->get_rooms()[maze];
//                    r->set_empty_private();
//                    r->synchronize();
//                }
//            }
            // Store not bisectable paves
            p->set_removed_rooms();
            m_paves_not_bisectable.push_back(p);
        }
    }

    /// Delete parent of bisected paves ///
    for(Pave* p:m_bisected_paves)
        delete(p);
}

void Graph::get_room_info(Maze *maze, const IntervalVector& position, vector<Pave*> &pave_list) const{
    m_tree->get_intersection_pave_outer(pave_list,position);
    for(Pave *p:pave_list){
        Room *r = p->get_rooms()[maze];
        cout << *r << endl;
    }
}


std::pair<IntervalVector, IntervalVector> Graph::bisect_largest_first(const IntervalVector &position){
    // Select dimensions to bisect
    bool one_possible = false;
    vector<bool> possible_dim;
    for(int dim = 0; dim<m_dim; dim++){
        if(position[dim].diam() > m_limit_bisection[dim]){
            possible_dim.push_back(true);
            one_possible = true;
        }
        else{
            possible_dim.push_back(false);
        }
    }
    if(!one_possible){ // If no-one possible make all possible
        for(int dim=0; dim<m_dim; dim++)
            possible_dim[dim] = true;
    }

    // Find largest dimension
    Vector diam = position.diam();
    int dim_max = 0;
    double max = 0;
    for(int i=0; i<m_dim; i++){
        double test = diam[i]*m_ratio_dimension[i];
        if((max<test) & (possible_dim[i])){
            max = test;
            dim_max = i;
        }
    }
    IntervalVector p1(position);
    IntervalVector p2(position);

    p1[dim_max] = Interval(position[dim_max].lb(), position[dim_max].mid());
    p2[dim_max] = Interval(position[dim_max].mid(), position[dim_max].ub());

    return std::make_pair(p1, p2);
}

}
