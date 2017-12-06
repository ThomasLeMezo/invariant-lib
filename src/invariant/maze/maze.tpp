#include "maze.h"

namespace invariant {

template<typename _Tp, typename _V>
Maze<_Tp, _V>::Maze(invariant::Domain<_Tp, _V> *domain, Dynamics *dynamics)
{
    m_domain = domain;
    m_dynamics = dynamics;

    m_subpaving = domain->get_subpaving();
    omp_init_lock(&m_deque_access);

    SmartSubPaving<_Tp, _V> *g = domain->get_subpaving();
    for(Pave<_Tp, _V>*p:g->get_paves()){
        Room<_Tp, _V> *r = new Room<_Tp, _V>(p, this, dynamics);
        p->add_room(r);
    }

    g->add_maze(this);
    m_empty = false;
}

template<typename _Tp, typename _V>
Maze<_Tp, _V>::~Maze(){
    omp_destroy_lock(&m_deque_access);
}

template<typename _Tp, typename _V>
void Maze<_Tp, _V>::init(){
    std::vector<Room<_Tp, _V> *> list_room_to_contract;
    invariant::Domain<_Tp, _V> *d = m_domain;
    d->contract_domain(this, list_room_to_contract);
}

template<typename _Tp, typename _V>
int Maze<_Tp, _V>::contract(){
    if(m_empty){
        std::cout << " ==> MAZE EMPTY" << std::endl;
        return 0;
    }
    // Domain contraction
    std::vector<Room<_Tp, _V> *> list_room_to_contract;
    invariant::Domain<_Tp, _V> *d = m_domain;
    double t_start = omp_get_wtime();
    d->contract_domain(this, list_room_to_contract);

    // Add Room to the Deque
    for(Room<_Tp, _V> *r:list_room_to_contract){
        if(!r->is_in_deque()){
            r->set_in_queue();
            add_to_deque(r);
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

    std::cout << " => sep : " << omp_get_wtime() - t_start << " deque size = " << m_deque_rooms.size() << std::endl;
    t_start = omp_get_wtime();
    int nb_operations = 0;

    // Propagation of contractions
    bool deque_empty = m_deque_rooms.empty();

    if(deque_empty){
        std::cout << " => MAZE EMPTY" << std::endl;
        m_empty = true;
        return 0;
    }

#pragma omp parallel
    {
#pragma omp single
        {
            while(!deque_empty){
#pragma omp task
                {
#pragma omp atomic
                            nb_operations++;
                    // Take one Room
                    omp_set_lock(&m_deque_access);
                    Room<_Tp, _V> *r = NULL;
                    if(!m_deque_rooms.empty()){
                        // To improve the efficiency, we start half of the thread on the back of the deque
                        // and the other on the front
                        if(omp_get_thread_num()%2==0){
                            r = m_deque_rooms.back();
                            m_deque_rooms.pop_back();
                        }
                        else{
                            r = m_deque_rooms.front();
                            m_deque_rooms.pop_front();
                        }
                        r->reset_deque(); // Room can be added again to the deque
                    }
                    omp_unset_lock(&m_deque_access);

                    if(r!=NULL){
                        r->lock_contraction();
                        // Contract
                        bool change = false;
                        change |= r->contract();

                        if(change){
                            // Analyse changes
                            std::vector<Room<_Tp, _V> *> rooms_to_update;
                            r->analyze_change(rooms_to_update);

                            // Synchronize
                            r->synchronize();

                            if(m_domain->get_init() == invariant::FULL_DOOR && r->is_empty())
                                r->set_removed();

                            // Add Rooms to the Deque
                            add_rooms(rooms_to_update);

                            // Increment operations
//#pragma omp atomic
//                            nb_operations++;
                        }

                        /// DEBUG
                        //                        VibesMaze v_maze("SmartSubPaving", m_subpaving, this);
                        //                        v_maze.setProperties(0, 0, 512, 512);
                        //                        v_maze.show();
                        //                        v_maze.show_room_info(this, test);

                        r->unlock_contraction();
                    }
                }

                omp_set_lock(&m_deque_access);
                deque_empty = m_deque_rooms.empty();
                omp_unset_lock(&m_deque_access);

                if(deque_empty){
#pragma omp taskwait
                    omp_set_lock(&m_deque_access);
                    deque_empty = m_deque_rooms.empty(); // New Rooms could have been added to the deque meanwhile taskwait
                    omp_unset_lock(&m_deque_access);
                }
            }
        }
    }

    std::cout << " => contractions (" << nb_operations << ") : " << omp_get_wtime() - t_start << " s";
    return nb_operations;
}

//template<typename _Tp, typename _V>
//void Maze<_Tp, _V>::contract_inter(Maze* maze_inter){
//    // Intersect this maze with other mazes
//    //    invariant::Domain *d = m_domain;
//    //    d->inter_maze(this);
//    if(is_escape_trajectories() && maze_inter->is_escape_trajectories()){

//        std::vector<Room *> room_list;
//        m_subpaving->get_tree()->get_all_child_rooms_not_empty(room_list, this);

//#pragma omp parallel for
//        for(size_t i=0; i<room_list.size(); i++){
//            Room *r = room_list[i];
//            Pave *p = r->get_pave();
//            Room *r_inter = p->get_rooms()[maze_inter];
//            if(r_inter->is_empty()){
//                r->set_empty_private();
//                r->synchronize();
//                r->set_removed();
//            }
//        }
//    }
//}

template<typename _Tp, typename _V>
bool Maze<_Tp, _V>::is_escape_trajectories(){
    if(m_espace_trajectories == false)
        return false;
    else{
        std::vector<Pave<_Tp, _V>*> pave_list_border;
        m_subpaving->get_tree()->get_border_paves(pave_list_border);

        for(Pave<_Tp, _V> *p:pave_list_border){
            Room<_Tp, _V> *r = p->get_rooms()[this];
            if(!r->is_removed()){
                for(Face<_Tp, _V> *f:p->get_faces_vector()){
                    if(f->is_border()){
                        Door<_Tp, _V> *d = f->get_doors()[this];
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

template<typename _Tp, typename _V>
void Maze<_Tp, _V>::add_rooms(const std::vector<Room<_Tp, _V> *>& list_rooms){
    for(Room<_Tp, _V> *r:list_rooms){
        if(!r->is_in_deque() && !r->is_removed()){
            r->set_in_queue();
            this->add_to_deque(r);
        }
    }
}


}
