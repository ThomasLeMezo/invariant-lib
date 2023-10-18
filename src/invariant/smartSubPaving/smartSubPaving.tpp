#include "smartSubPaving.h"

namespace invariant {
template<typename _TpR, typename _TpF, typename _TpD>
SmartSubPaving<_TpR,_TpF,_TpD>::SmartSubPaving(const ibex::IntervalVector &space):
    m_position(space)
{
    m_dim = (unsigned char) space.size();
    omp_init_lock(&m_write_add_pave);

    // Create search space Pave
    invariant::Pave<_TpR,_TpF,_TpD>* p = new invariant::Pave<_TpR,_TpF,_TpD>(space, this);
    m_paves.push_back(p);

    // Analyze faces (border)
    for(Pave<_TpR,_TpF,_TpD> *p:m_paves)
        p->analyze_border();
    for(Pave<_TpR,_TpF,_TpD> *p:m_paves_removed)
        p->analyze_border();

    // Root of the pave node tree
    m_tree = new Pave_node<_TpR,_TpF,_TpD>(p);
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

template<typename _TpR, typename _TpF, typename _TpD>
SmartSubPaving<_TpR,_TpF,_TpD>::~SmartSubPaving(){
    for(Pave<_TpR,_TpF,_TpD> *p:m_paves){
        if(p!=nullptr)
            delete(p);
    }
    delete(m_tree);
    omp_destroy_lock(&m_write_add_pave);
}

template<typename _TpR, typename _TpF, typename _TpD>
void SmartSubPaving<_TpR,_TpF,_TpD>::delete_pave(int id){
    delete(m_paves[id]);
    m_paves[id] = nullptr;
}

template<typename _TpR, typename _TpF, typename _TpD>
void SmartSubPaving<_TpR,_TpF,_TpD>::serialize(std::ofstream& binFile) const{
    // unsigned char    dimension
    // size_t           number of paves
    // IntervalVector   position
    // [...] Paves of the graph

    binFile.write((const char*)&m_dim, sizeof(unsigned char)); // dimension
    size_t size = m_paves.size();
    binFile.write((const char*)&size, sizeof(size_t)); // Number of paves
    serializeIntervalVector(binFile, m_position);

    size_t cpt = 0;
    for(Pave<_TpR,_TpF,_TpD> *p:m_paves){
        p->set_serialization_id(cpt); cpt++;
        p->serialize(binFile);
    }
}

template<typename _TpR, typename _TpF, typename _TpD>
void SmartSubPaving<_TpR,_TpF,_TpD>::deserialize(std::ifstream& binFile){
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
        Pave<_TpR,_TpF,_TpD> *p = new Pave<_TpR,_TpF,_TpD>(this);
        m_paves.push_back(p);
    }
    for(size_t i=0; i<number_pave_const; i++){
        Pave<_TpR,_TpF,_TpD> *p = m_paves[i];
        p->deserialize(binFile);
    }
}

template<typename _TpR, typename _TpF, typename _TpD>
const bool SmartSubPaving<_TpR,_TpF,_TpD>::is_equal(const SmartSubPaving<_TpR,_TpF,_TpD>& g) const{
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

template<typename _TpR, typename _TpF, typename _TpD>
void SmartSubPaving<_TpR,_TpF,_TpD>::bisect(){
    std::cout << " => bisecting ";
    double time_start = omp_get_wtime();
    std::cout << " (from " << m_paves.size();
    std::vector<Pave<_TpR,_TpF,_TpD>*> bisectable_paves = m_paves;
    std::vector<Pave<_TpR,_TpF,_TpD>*> bisected_paves;
    m_paves.clear();

    // Hybrid reset
    for(Maze<_TpR,_TpF,_TpD> *maze:m_mazes){
        maze->reset_hybrid_room_list();
    }

#pragma omp parallel
    {
        Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_TpD>();
#pragma omp for
        for(size_t i = 0; i<bisectable_paves.size(); i++){
            Pave<_TpR,_TpF,_TpD> *p = bisectable_paves[i];

            if((m_bisection_tree==nullptr && p->request_bisection())
               || (m_bisection_tree!=nullptr && (m_bisection_tree->eval_bisection(p)))){
                if(p->bisect_step_one()){
#pragma omp critical
                    {
                        bisected_paves.push_back(p);
                    }
                }
            }
            else{
                bool all_removed = p->set_removed_rooms();
                if(all_removed){ // Store not bisectable paves (for all mazes)
#pragma omp critical
                    {
                        m_paves_removed.push_back(p);
                    }
                }
                else{
                    this->add_paves(p);
                }
            }
        }

#pragma omp for
        for(size_t i = 0; i<bisected_paves.size(); i++){
            Pave<_TpR,_TpF,_TpD> *p = bisected_paves[i];
            p->bisect_step_two();
        }

#pragma omp for
        for(size_t i = 0; i<bisected_paves.size(); i++){
            Pave<_TpR,_TpF,_TpD> *p = bisected_paves[i];
            delete(p);
        }
        delete_thread_init<_TpD>(thread_init);
    }

    // Reset maze
    for(Maze<_TpR,_TpF,_TpD> *maze:m_mazes){
        maze->reset_nb_operations();
    }

    // Hybrid discover
    for(Maze<_TpR,_TpF,_TpD> *maze:m_mazes){
        maze->discover_hybrid_rooms();
    }

    m_bisection_step++;
    std::cout << " to " << m_paves.size() << ") - ";
    std::cout << omp_get_wtime() - time_start << "s" <<  std::endl;
}

template<typename _TpR, typename _TpF, typename _TpD>
void SmartSubPaving<_TpR,_TpF,_TpD>::bisect_monothread(){
    std::cout << " => bisecting ";
    double time_start = omp_get_wtime();

    std::vector<Pave<_TpR,_TpF,_TpD>*> bisectable_paves = m_paves;
    m_paves.clear();

    /// Bisect the graph ///
    while(bisectable_paves.size()>0){
        Pave<_TpR,_TpF,_TpD> *p = bisectable_paves.back();
        bisectable_paves.pop_back();

        if(p->request_bisection()){
            if(p->bisect_monothread()) // bisected added to m_paves & update mazes
                delete(p);
        }
        else{
            // Store not bisectable paves
            p->set_removed_rooms();
            m_paves_removed.push_back(p);
        }
    }

    // Reset maze
    for(Maze<_TpR,_TpF,_TpD> *maze:m_mazes){
        maze->reset_nb_operations();
    }

    std::cout << omp_get_wtime() - time_start << "s" <<  std::endl;
}

template<typename _TpR, typename _TpF, typename _TpD>
void SmartSubPaving<_TpR,_TpF,_TpD>::get_room_info(Maze<_TpR,_TpF,_TpD> *maze, const ibex::IntervalVector& position, std::vector<Pave<_TpR,_TpF,_TpD>*> &pave_list) const{
    m_tree->get_intersection_pave_outer(pave_list,position);
    for(Pave<_TpR,_TpF,_TpD> *p:pave_list){
        Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[maze];
        std::cout << *r << std::endl;
    }
}


template<typename _TpR, typename _TpF, typename _TpD>
std::pair<ibex::IntervalVector, ibex::IntervalVector> SmartSubPaving<_TpR,_TpF,_TpD>::bisect_largest_first(const ibex::IntervalVector &position){
    // Select dimensions to bisect
    bool one_possible = false;
    std::vector<bool> possible_dim;
    for(int dim = 0; dim<m_dim; dim++){
        if(position[dim].diam() > m_limit_bisection[dim]){
            possible_dim.push_back(true);
            one_possible = true;
        }
        else
            possible_dim.push_back(false);
    }

    // Find largest dimension
    ibex::Vector diam = position.diam();
    int dim_max = 0;
    double max = 0;
    for(int i=0; i<m_dim; i++){
        double test = diam[i]*m_ratio_dimension[i];
        if((max<test) & (possible_dim[i] || !one_possible)){
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
    else if(m_bisection_strategy[dim_max]==BISECTION_LB_UB && position[dim_max].diam()>2*m_bisection_strategy_slice[dim_max]){
        if(position[dim_max].mid()-m_position[dim_max].lb()>m_position[dim_max].ub()-position[dim_max].mid()){
            p1[dim_max] = ibex::Interval(position[dim_max].lb(), position[dim_max].ub()-m_bisection_strategy_slice[dim_max]);
            p2[dim_max] = ibex::Interval(position[dim_max].ub()-m_bisection_strategy_slice[dim_max], position[dim_max].ub());
        }
        else{
            p1[dim_max] = ibex::Interval(position[dim_max].lb(), position[dim_max].lb()+m_bisection_strategy_slice[dim_max]);
            p2[dim_max] = ibex::Interval(position[dim_max].lb()+m_bisection_strategy_slice[dim_max], position[dim_max].ub());
        }
    }
    else{
        p1[dim_max] = ibex::Interval(position[dim_max].lb(), position[dim_max].mid());
        p2[dim_max] = ibex::Interval(position[dim_max].mid(), position[dim_max].ub());
    }

    return std::make_pair(p1, p2);
}

template<typename _TpR, typename _TpF, typename _TpD>
bool SmartSubPaving<_TpR,_TpF,_TpD>::bisection_limit_reach(const ibex::IntervalVector &position){
    for(int dim = 0; dim<m_dim; dim++){
        if(position[dim].diam() >= m_limit_bisection[dim])
            return false;
    }
    return true;
}

}
