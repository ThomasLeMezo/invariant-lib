
#include "maze.h"

namespace invariant{
// For shared library (instanciation)
template class Maze<Parma_Polyhedra_Library::C_Polyhedron, Parma_Polyhedra_Library::Generator_System>;
template class Maze<ibex::IntervalVector, ibex::IntervalVector>;

}
