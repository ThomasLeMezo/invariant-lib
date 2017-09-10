#include "maze.h"

#include "../../graphics/vibes_graph.h"

using namespace std;
using namespace ibex;
namespace invariant {

Maze::Maze(invariant::Domain *domain, Dynamics *dynamics, MazeSens maze_sens, MazeType maze_type)
{
    m_maze_sens = maze_sens;
    m_maze_type = maze_type;
    m_domain = domain;
    m_dynamics = dynamics;
    m_graph = domain->get_graph();
    omp_init_lock(&m_deque_access);

    Graph *g = domain->get_graph();
    for(Pave*p:g->get_paves()){
        Room *r = new Room(p, this, dynamics);
        p->add_room(r);
    }

    g->add_maze(this);
}

Maze::~Maze(){
    omp_destroy_lock(&m_deque_access);
}

void Maze::init(){
    vector<Room *> list_room_to_contract;
    invariant::Domain *d = m_domain;
    d->contract_domain(this, list_room_to_contract);
}

int Maze::contract(){
    // Domain contraction
    vector<Room *> list_room_to_contract;
    invariant::Domain *d = m_domain;
    double t_start = omp_get_wtime();
    d->contract_domain(this, list_room_to_contract);

    // Add Room to the Deque
    for(Room *r:list_room_to_contract){
        if(!r->is_in_deque()){
            r->set_in_queue();
            add_to_deque(r);
        }
    }

    /// DEBUG
    //    if(m_maze_type == MAZE_PROPAGATOR){
    //        Vibes_Graph v_graph("graph_debug", m_graph, this);
    //        v_graph.setProperties(0, 0, 512, 512);
    //        v_graph.show();
    //        IntervalVector test(2);
    //        test[0] = Interval(-2);
    //        test[1] = Interval(2);
    //        v_graph.show_room_info(this, test);
    //        cout << "debug graph_debug" << endl;
    //    }

    cout << " => sep : " << omp_get_wtime() - t_start << " deque size = " << m_deque_rooms.size() << endl;
    t_start = omp_get_wtime();
    int nb_operations = 0;

    // Propagation of contractions
    bool deque_empty = m_deque_rooms.empty();

    if(deque_empty){
        cout << " => MAZE EMPTY" << endl;
        return 0;
    }

#pragma omp parallel //num_threads(1)
    {
#pragma omp single
        {
            while(!deque_empty){
#pragma omp task
                {
                    // Take one Room
                    omp_set_lock(&m_deque_access);
                    Room *r = NULL;
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
                            vector<Room *> rooms_update;
                            r->analyze_change(rooms_update);

                            // Synchronize
                            r->synchronize_doors();

                            if(m_maze_type == MAZE_CONTRACTOR && r->is_empty())
                                r->set_removed();

                            // Add Rooms to the Deque
                            add_rooms(rooms_update);

                            // Increment operations
#pragma omp atomic
                            nb_operations++;
                        }

                        /// DEBUG
                        //                        Vibes_Graph v_graph("graph", m_graph, this);
                        //                        v_graph.setProperties(0, 0, 512, 512);
                        //                        v_graph.show();
                        //                        v_graph.show_room_info(this, test);

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

    cout << " => contractions : " << omp_get_wtime() - t_start << endl;
    return nb_operations;
}

void Maze::contract_inter(Maze* maze_inter){
    // Intersect this maze with other mazes
    //    invariant::Domain *d = m_domain;
    //    d->inter_maze(this);
    if(is_escape_trajectories() && maze_inter->is_escape_trajectories()){

        std::vector<Room *> room_list;
        m_graph->get_tree()->get_all_child_rooms_not_empty(room_list, this);

#pragma omp parallel for
        for(size_t i=0; i<room_list.size(); i++){
            Room *r = room_list[i];
            Pave *p = r->get_pave();
            Room *r_inter = p->get_rooms()[maze_inter];
            if(r_inter->is_empty()){
                r->set_empty_private();
                r->synchronize();
            }
        }
    }
}

bool Maze::is_escape_trajectories(){
    if(m_espace_trajectories == false)
        return false;
    else{
        std::vector<Pave*> pave_list_border;
        m_graph->get_tree()->get_border_paves(pave_list_border);

        for(Pave *p:pave_list_border){
            Room *r = p->get_rooms()[this];
            if(!(r->is_removed() || r->is_empty()))
                return true;
        }
    }
    return true;
}

void Maze::add_rooms(const vector<Room *>& list_rooms){
    for(Room *r:list_rooms){
        if(!r->is_in_deque()){
            r->set_in_queue();
            this->add_to_deque(r);
        }
    }
}


}
