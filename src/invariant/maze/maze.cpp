#include "maze.h"

using namespace std;
using namespace ibex;
namespace invariant {

Maze::Maze(invariant::Domain *domain, Dynamics *dynamics, MazeSens maze_sens, MazeType maze_type)
{
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

    m_maze_sens = maze_sens;
    m_maze_type = maze_type;
}

Maze::~Maze(){
    omp_destroy_lock(&m_deque_access);
}

int Maze::contract(){
    // Domain contraction
    vector<Room *> list_room_to_contract;
    invariant::Domain *d = m_domain;
    double t_start = omp_get_wtime();
    d->contract_separator(this, list_room_to_contract);

    // Add Room to the Deque
    for(Room *r:list_room_to_contract){
        if(!r->is_in_deque()){
            r->set_in_queue();
            add_to_deque(r);
        }
    }

    cout << " => sep : " << omp_get_wtime() - t_start << endl;
    t_start = omp_get_wtime();
    int nb_operations = 0;

    // Propagation of contractions
    bool deque_empty = m_deque_rooms.empty();

#pragma omp parallel
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

                            // Add Rooms to the Deque
                            add_rooms(rooms_update);

                            // Increment operations
                            #pragma omp atomic
                            nb_operations++;
                        }
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
    cout << " => sivia : " << omp_get_wtime() - t_start << endl;
    return nb_operations;
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
