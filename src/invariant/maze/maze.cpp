
#include "maze.h"

namespace invariant{
// For shared library (instanciation)
template class Maze<Parma_Polyhedra_Library::C_Polyhedron>;
template class Maze<ibex::IntervalVector>;

}
