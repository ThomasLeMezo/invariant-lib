#include "maze.h"

using namespace std;
using namespace ibex;
namespace invariant {

Maze::Maze(Domain *domain, Dynamics *dynamics)
{
    m_domain = domain;
    m_dynamics = dynamics;
}

void Maze::contract(){
    // Domain contractions
    vector<Room *> list_room_to_contract;
    m_domain->contract_separator(this, list_room_to_contract);

    // Propagation of contractions
}

}
