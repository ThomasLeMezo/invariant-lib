#include "maze.h"

namespace invariant {

template<typename _TpR, typename _TpF, typename _TpD>
Maze<_TpR,_TpF,_TpD>::Maze(invariant::Domain<_TpR,_TpF,_TpD> *domain, Dynamics *dynamics)
{
    m_domain = domain;
    m_dynamics = dynamics;

    m_subpaving = domain->get_subpaving();
    omp_init_lock(&m_deque_access);
    omp_init_lock(&m_initial_rooms_access);
    omp_init_lock(&m_hybrid_rooms_access);

    std::cout << "oui ?\n";
    SmartSubPaving<_TpR,_TpF,_TpD> *g = domain->get_subpaving();
    std::cout << "non !\n";
    for(Pave<_TpR,_TpF,_TpD>*p:g->get_paves()){
        Room<_TpR,_TpF,_TpD> *r = new Room<_TpR,_TpF,_TpD>(p, this, dynamics);
        std::cout << "construction salle du pave " << p->get_position() << "\n";
        p->add_room(r);
    }

    discover_hybrid_rooms();

    g->add_maze(this);
    m_empty = false;
}

template<typename _TpR, typename _TpF, typename _TpD>
Maze<_TpR,_TpF,_TpD>::~Maze(){
    omp_destroy_lock(&m_deque_access);
    omp_destroy_lock(&m_initial_rooms_access);
    omp_destroy_lock(&m_hybrid_rooms_access);
}

template<typename _TpR, typename _TpF, typename _TpD>
int Maze<_TpR,_TpF,_TpD>::contract(size_t nb_operations){
    if(m_empty){
        std::cout << " ==> MAZE EMPTY (begin)" << std::endl;
        return 0;
    }
    double t_start = omp_get_wtime();

    // Domain contraction
    if(m_nb_operations == 0){
        m_deque_rooms.clear();
        std::vector<Room<_TpR,_TpF,_TpD> *> list_room_to_contract;
        invariant::Domain<_TpR,_TpF,_TpD> *d = m_domain;
        if(m_enable_contract_domain)
            d->contract_domain(this, list_room_to_contract);
        else{
            m_subpaving->get_tree()->get_all_child_rooms(list_room_to_contract, this);
        }

        // Add Room to the Deque
        for(Room<_TpR,_TpF,_TpD> *r:list_room_to_contract){
            if(r->set_in_queue()){
                add_to_deque(r);
            }
        }
    }

    /// DEBUG
    //    if(m_maze_type == MAZE_WALL){
    //        VibesMaze v_maze("graph_debug", m_subpaving, this);
    //        v_maze.setProperties(0, 0, 512, 512);
    //        v_maze.show();
    //        IntervalVector test(2);
    //        test[0] = Interval(-2);
    //        test[1] = Interval(2);
    //        v_maze.show_room_info(this, test);
    //        std::cout << "debug graph_debug" << std::endl;
    //    }

    std::cout << " => domain contraction : " << omp_get_wtime() - t_start << "s, " << m_deque_rooms.size() << " rooms in deque" << std::endl;
    t_start = omp_get_wtime();
    int nb_deque = 0;

    // Propagation of contractions
    bool stop_contraction = m_deque_rooms.empty();

    if(stop_contraction && m_contraction_step!=0){
        std::cout << " => [WARNING] MAZE EMPTY" << std::endl;
        m_empty = true;
        return 0;
    }
    if((nb_operations!=0 && m_nb_operations>=nb_operations)){
        std::cout << " => [WARNING] Not enough step allowed" << std::endl;
        return 0;
    }
    omp_lock_t lock_nb_operations;
    omp_init_lock(&lock_nb_operations);

#pragma omp parallel shared(stop_contraction)
    {
        Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_TpD>();
#pragma omp single
        {
            while(!stop_contraction){
#pragma omp task
                {
                    // Take one Room
                    Room<_TpR,_TpF,_TpD> *r = nullptr;
                    omp_set_lock(&m_deque_access);

                    if(!m_deque_rooms.empty() && !stop_contraction){
                        // To improve the efficiency, we start half of the thread on the back of the deque
                        // and the other on the front
                        if(omp_get_thread_num()%2==1){
                            r = m_deque_rooms.back();
                            m_deque_rooms.pop_back();
                        }
                        else{
                            r = m_deque_rooms.front();
                            if(r->get_pave()->get_position().size()>10)
                                std::cout << "WARNING dimension > 10" << std::endl;
                            m_deque_rooms.pop_front();
                        }
                        r->reset_deque(); // Room can be added again to the deque
                        nb_deque++;
                    }
                    omp_unset_lock(&m_deque_access);

                    if(r!=nullptr){ // Room can be empty (because m_deque_rooms need lock)
                        r->lock_contraction();
                        // Contract
                        bool change = false;
                        change |= r->contract();

                        if(change){
                            // Analyse changes
                            std::vector<Room<_TpR,_TpF,_TpD> *> rooms_to_update;
                            r->analyze_change(rooms_to_update);

                            // Synchronize
                            r->synchronize();

                            if(m_domain->get_init() == invariant::FULL_DOOR && r->is_empty())
                                r->set_removed();

                            // Add Rooms to the Deque
                            add_rooms(rooms_to_update);

                            // Increment operations
                            omp_set_lock(&lock_nb_operations);
                            m_nb_operations++;
                            omp_unset_lock(&lock_nb_operations);
                        }

                        r->unlock_contraction();
                    }
                }

                omp_set_lock(&m_deque_access);
                omp_set_lock(&lock_nb_operations);
                stop_contraction = m_deque_rooms.empty() || (nb_operations!=0 && m_nb_operations>=nb_operations);
                #pragma omp flush(stop_contraction)
                omp_unset_lock(&lock_nb_operations);
                omp_unset_lock(&m_deque_access);

                if(stop_contraction){
                    #pragma omp taskwait
                    omp_set_lock(&m_deque_access);
                    omp_set_lock(&lock_nb_operations);
                    // Evaluate a second time to verify if no new rooms where added after all thread have finish (taskwait)
                    stop_contraction = m_deque_rooms.empty() || (nb_operations!=0 && m_nb_operations>=nb_operations) ; // New Rooms could have been added to the deque meanwhile taskwait
                    omp_unset_lock(&lock_nb_operations);
                    omp_unset_lock(&m_deque_access);
                }
            }
        }
        delete_thread_init<_TpD>(thread_init);
    }
    omp_destroy_lock(&lock_nb_operations);
    reset_initial_room_list();

    std::cout << " => contractions : " << omp_get_wtime() - t_start << " s, with " << m_nb_operations << "/" << nb_deque << " operations" <<  std::endl;
    m_contraction_step++;
    m_contract_once = true;

    int return_remain_nb_operations = m_nb_operations; //std::max(0, (int)(m_nb_operations)-(int)nb_operations); // To be improved (with ssize_t ?)
    if(m_deque_rooms.empty())
        m_nb_operations = 0;
    else
        std::cout << " => limit number of operation reached (" << nb_operations << ")" << std::endl;
    return return_remain_nb_operations;
}

template<typename _TpR, typename _TpF, typename _TpD>
void Maze<_TpR,_TpF,_TpD>::reset_nb_operations(){
    m_nb_operations = 0;
    m_contract_once = false;
    m_deque_rooms.clear();
}

template<typename _TpR, typename _TpF, typename _TpD>
bool Maze<_TpR,_TpF,_TpD>::is_escape_trajectories(){
    if(m_espace_trajectories == false)
        return false;
    else{
        std::vector<Pave<_TpR,_TpF,_TpD>*> pave_list_border;
        m_subpaving->get_tree()->get_border_paves(pave_list_border);

        for(Pave<_TpR,_TpF,_TpD> *p:pave_list_border){
            Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[this];
            if(!r->is_removed()){
                for(Face<_TpR,_TpF,_TpD> *f:p->get_faces_vector()){
                    if(f->is_border()){
                        Door<_TpR,_TpF,_TpD> *d = f->get_doors()[this];
                        if(!d->is_empty())
                            return true;
                    }
                }
            }
        }
        m_espace_trajectories = false;
        return false;
    }
}

template<typename _TpR, typename _TpF, typename _TpD>
bool Maze<_TpR,_TpF,_TpD>::is_enable_father_hull() const{
    return m_enable_father_hull;
}

template<typename _TpR, typename _TpF, typename _TpD>
void Maze<_TpR,_TpF,_TpD>::add_rooms(const std::vector<Room<_TpR,_TpF,_TpD> *>& list_rooms){
    for(Room<_TpR,_TpF,_TpD> *r:list_rooms){
        bool valid = r->set_in_queue();
        if(valid)
            this->add_to_deque(r);
    }
}

template<typename _TpR, typename _TpF, typename _TpD>
ibex::IntervalVector Maze<_TpR,_TpF,_TpD>::get_bounding_box(){
    ibex::IntervalVector result(m_subpaving->dim(), ibex::Interval::EMPTY_SET);
    m_subpaving->get_tree()->get_bounding_fullness(this, result);
    return result;
}

template<typename _TpR, typename _TpF, typename _TpD>
void Maze<_TpR,_TpF,_TpD>::compute_vector_field(){
    for(Pave<_TpR,_TpF,_TpD>*p:m_subpaving->get_paves()){
        Room<_TpR,_TpF,_TpD> *r = p->get_rooms()[this];
        r->compute_vector_field();
    }
}

template<typename _TpR, typename _TpF, typename _TpD>
void Maze<_TpR,_TpF,_TpD>::discover_hybrid_rooms(){
#pragma omp for
        for(size_t i = 0; i<m_hybrid_rooms_list.size(); i++){
            Room<_TpR,_TpF,_TpD>* r=m_hybrid_rooms_list[i];
            r->reset_hybrid_doors();
        }
#pragma omp for
        for(size_t i = 0; i<m_hybrid_rooms_list.size(); i++){
            Room<_TpR,_TpF,_TpD>* r=m_hybrid_rooms_list[i];
            r->discover_hybrid_room();
        }
}

}
