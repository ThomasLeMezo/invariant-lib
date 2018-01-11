#include "smartSubPaving.h"

namespace invariant {
template<typename _Tp>
SmartSubPaving<_Tp>::SmartSubPaving(const ibex::IntervalVector &space):
    m_position(space)
{
    m_dim = (unsigned char) space.size();
    omp_init_lock(&m_write_add_pave);

    // Create search space Pave
    invariant::Pave<_Tp>* p = new invariant::Pave<_Tp>(space, this);
    m_paves.push_back(p);

    // Analyze faces (border)
    for(Pave<_Tp> *p:m_paves)
        p->analyze_border();
    for(Pave<_Tp> *p:m_paves_not_bisectable)
        p->analyze_border();

    // Root of the pave node tree
    m_tree = new Pave_node<_Tp>(p);
    p->set_pave_node(m_tree);

    // Compute ratio dimension
    for(int dim=0; dim<m_dim; dim++){
        double diam = space[dim].diam();
        m_ratio_dimension.push_back(1.0/diam);
        m_limit_bisection.push_back(0.0);
    }

    for(int dim=0; dim<m_dim; dim++){
        m_bisection_strategy.push_back(BISECTION_STANDARD);
        m_bisection_strategy_slice.push_back(0.5);
    }
}

template<typename _Tp>
SmartSubPaving<_Tp>::~SmartSubPaving(){
    for(Pave<_Tp> *p:m_paves){
        if(p!=nullptr)
            delete(p);
    }
    delete(m_tree);
    omp_destroy_lock(&m_write_add_pave);
}

template<typename _Tp>
void SmartSubPaving<_Tp>::delete_pave(int id){
    delete(m_paves[id]);
    m_paves[id] = nullptr;
}

template<typename _Tp>
void SmartSubPaving<_Tp>::serialize(std::ofstream& binFile) const{
    // unsigned char    dimension
    // size_t           number of paves
    // IntervalVector   position
    // [...] Paves of the graph

    binFile.write((const char*)&m_dim, sizeof(unsigned char)); // dimension
    size_t size = m_paves.size();
    binFile.write((const char*)&size, sizeof(size_t)); // Number of paves
    serializeIntervalVector(binFile, m_position);

    size_t cpt = 0;
    for(Pave<_Tp> *p:m_paves){
        p->set_serialization_id(cpt); cpt++;
        p->serialize(binFile);
    }
}

template<typename _Tp>
void SmartSubPaving<_Tp>::deserialize(std::ifstream& binFile){
    if(m_paves.size()!=0){
        throw std::runtime_error("in [graph.cpp/deserialize] SmartSubPaving is not empty");
        return;
    }

    binFile.read((char*)&m_dim, sizeof(unsigned char));
    size_t number_pave;
    binFile.read((char*)&number_pave, sizeof(size_t));
    m_position = deserializeIntervalVector(binFile);

    const size_t number_pave_const = number_pave;
    for(size_t i=0; i<number_pave_const; i++){
        Pave<_Tp> *p = new Pave<_Tp>(this);
        m_paves.push_back(p);
    }
    for(size_t i=0; i<number_pave_const; i++){
        Pave<_Tp> *p = m_paves[i];
        p->deserialize(binFile);
    }
}

template<typename _Tp>
const bool SmartSubPaving<_Tp>::is_equal(const SmartSubPaving<_Tp>& g) const{
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

template<typename _Tp>
void SmartSubPaving<_Tp>::bisect(){
    std::cout << " => bisecting ";
    double time_start = omp_get_wtime();

    std::vector<Pave<_Tp>*> bisectable_paves = m_paves;
    m_paves.clear();

    /// Bisect the graph ///
    while(bisectable_paves.size()>0){
        Pave<_Tp> *p = bisectable_paves.back();
        bisectable_paves.pop_back();

        if(p->request_bisection()){
            if(p->bisect()) // bisected added to m_paves & update mazes
                delete(p);
        }
        else{
            // Store not bisectable paves
            p->set_removed_rooms();
            m_paves_not_bisectable.push_back(p);
        }
    }

    // Reset maze
    for(Maze<_Tp> *maze:m_mazes){
        maze->reset_nb_operations();
    }

    std::cout << omp_get_wtime() - time_start << "s" <<  std::endl;
}

template<typename _Tp>
void SmartSubPaving<_Tp>::get_room_info(Maze<_Tp> *maze, const ibex::IntervalVector& position, std::vector<Pave<_Tp>*> &pave_list) const{
    m_tree->get_intersection_pave_outer(pave_list,position);
    for(Pave<_Tp> *p:pave_list){
        Room<_Tp> *r = p->get_rooms()[maze];
        std::cout << *r << std::endl;
    }
}


template<typename _Tp>
std::pair<ibex::IntervalVector, ibex::IntervalVector> SmartSubPaving<_Tp>::bisect_largest_first(const ibex::IntervalVector &position){
    // Select dimensions to bisect
    bool one_possible = false;
    std::vector<bool> possible_dim;
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
    ibex::Vector diam = position.diam();
    int dim_max = 0;
    double max = 0;
    for(int i=0; i<m_dim; i++){
        double test = diam[i]*m_ratio_dimension[i];
        if((max<test) & (possible_dim[i])){
            max = test;
            dim_max = i;
        }
    }
    ibex::IntervalVector p1(position);
    ibex::IntervalVector p2(position);

    if(m_bisection_strategy[dim_max]==BISECTION_LB && position[dim_max].diam()>2*m_bisection_strategy_slice[dim_max]){ // Test bisection strategy for time
        p1[dim_max] = ibex::Interval(position[dim_max].lb(), position[dim_max].lb()+m_bisection_strategy_slice[dim_max]);
        p2[dim_max] = ibex::Interval(position[dim_max].lb()+m_bisection_strategy_slice[dim_max], position[dim_max].ub());
    }
    else if(m_bisection_strategy[dim_max]==BISECTION_UB && position[dim_max].diam()>2*m_bisection_strategy_slice[dim_max]){ // Test bisection strategy for time
        p1[dim_max] = ibex::Interval(position[dim_max].lb(), position[dim_max].ub()-m_bisection_strategy_slice[dim_max]);
        p2[dim_max] = ibex::Interval(position[dim_max].ub()-m_bisection_strategy_slice[dim_max], position[dim_max].ub());
    }
    else{
        p1[dim_max] = ibex::Interval(position[dim_max].lb(), position[dim_max].mid());
        p2[dim_max] = ibex::Interval(position[dim_max].mid(), position[dim_max].ub());
    }

    return std::make_pair(p1, p2);
}

template<typename _Tp>
bool SmartSubPaving<_Tp>::bisection_limit_reach(const ibex::IntervalVector &position){
    for(int dim = 0; dim<m_dim; dim++){
        if(position[dim].diam() >= m_limit_bisection[dim])
            return false;
    }
    return true;
}

}
