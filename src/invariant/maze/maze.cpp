#include "maze.h"

using namespace std;
using namespace ibex;
namespace invariant {

Maze::Maze(invariant::Domain *domain, Dynamics *dynamics)
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
}

Maze::~Maze(){
    omp_destroy_lock(&m_deque_access);
}

int Maze::contract(){
    // Domain contraction
    vector<Room *> list_room_to_contract;
    m_domain->contract_separator(this, list_room_to_contract);

    // Add Room to the Deque
    for(Room *r:list_room_to_contract){
        if(!r->is_in_deque()){
            r->set_in_queue();
            add_to_deque(r);
        }
    }

    int nb_operations = 0;

    // Propagation of contractions
    while(m_deque_rooms.size()>0){

        // Take one Room
        omp_set_lock(&m_deque_access);
        Room *r = m_deque_rooms.front();
        m_deque_rooms.pop_front();
        r->reset_deque(); // Room can be added again to the deque
        omp_unset_lock(&m_deque_access);

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
            nb_operations++;
        }
    }
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
