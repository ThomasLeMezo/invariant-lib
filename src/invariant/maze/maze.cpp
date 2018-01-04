#include "maze.h"

namespace invariant{
// For shared library (instanciation)
template class Maze<Parma_Polyhedra_Library::C_Polyhedron>;
template class Maze<ibex::IntervalVector>;

Parma_Polyhedra_Library::Thread_Init* initialize_thread(){
    Parma_Polyhedra_Library::Thread_Init *thread_init = NULL;
    bool thread_init_valid = true;
    #pragma omp master
    {
        thread_init_valid = false;
    }
    if(thread_init_valid)
        thread_init = new Parma_Polyhedra_Library::Thread_Init();
    return thread_init;
}

void delete_thread_init(Parma_Polyhedra_Library::Thread_Init* thread_init){
    if(thread_init!=NULL)
        delete(thread_init);
}

}

