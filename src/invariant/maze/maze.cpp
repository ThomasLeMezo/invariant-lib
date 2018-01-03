
#include "maze.h"

namespace invariant{
// For shared library (instanciation)
template class Maze<Parma_Polyhedra_Library::C_Polyhedron>;
template class Maze<ibex::IntervalVector>;

template <>
void Maze<ibex::IntervalVector>::initialize_thread(){

}

template <>
void Maze<Parma_Polyhedra_Library::C_Polyhedron>::initialize_thread(){
    //Parma_Polyhedra_Library::Thread_Init thread_init;
}


}
