#include "domain.h"

using namespace ibex;
using namespace std;
namespace invariant {

Domain::Domain(Graph *graph){
    m_graph = graph;
}

void Domain::contract(Door &door){
    IntervalVector input(door.get_input_private());
    IntervalVector output(door.get_output_private());
}

void Domain::contract(IntervalVector &iv,
                      Maze *maze,
                      const vector<IntervalVector> &remove_boxes,
                      const vector<pair<Maze*, bool>> &remove_mazes,
                      const vector<Ctc*> &remove_contractor)
{
    if(iv.is_empty())
            return;

    // Boxes
    for(const IntervalVector &box:remove_boxes){

    }
    if(iv.is_empty())
            return;

    // Contractors
    for(Ctc* ctc:remove_contractor){
        ctc->contract(iv);
    }
    if(iv.is_empty())
        return;

    // Mazes
    for(const std::pair<Maze*, bool> &pair:remove_mazes){
//        IntervalVector door_maze = ;
    }
}
}
