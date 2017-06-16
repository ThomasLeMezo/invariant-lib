#ifndef MAZE_H
#define MAZE_H

#include "graph.h"
#include <ibex.h>

namespace invariant {
class Graph; // declared only for friendship
class Maze
{
public:
    Maze(Graph *g, std::vector<ibex::Function*> f_vect);

private:
    std::vector<ibex::Function*> m_f_vect;


};
}
#endif // MAZE_H
