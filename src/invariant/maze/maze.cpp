#include "maze.h"

namespace invariant{
// For shared library (instanciation)
template class Maze<Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron,Parma_Polyhedra_Library::C_Polyhedron>;
template class Maze<ibex::IntervalVector,ibex::IntervalVector,ibex::IntervalVector>;
template class Maze<ibex::IntervalVector,ExpVF,ExpPoly>;

template<>
Parma_Polyhedra_Library::Thread_Init* initialize_thread<ppl::C_Polyhedron>(){
    Parma_Polyhedra_Library::Thread_Init *thread_init = nullptr;
    bool thread_init_valid = true;
    #pragma omp master
    {
        thread_init_valid = false;
    }
    if(thread_init_valid)
        thread_init = new Parma_Polyhedra_Library::Thread_Init();
    return thread_init;
}

template<>
Parma_Polyhedra_Library::Thread_Init* initialize_thread<ibex::IntervalVector>(){
    return nullptr;
}

template<>
Parma_Polyhedra_Library::Thread_Init* initialize_thread<ExpPoly>(){
    return nullptr;
}

template <>
void delete_thread_init<ppl::C_Polyhedron>(Parma_Polyhedra_Library::Thread_Init* thread_init){
    if(thread_init!=nullptr)
        delete(thread_init);
}

template <>
void delete_thread_init<ibex::IntervalVector>(Parma_Polyhedra_Library::Thread_Init* thread_init){
}

template <>
void delete_thread_init<ExpPoly>(Parma_Polyhedra_Library::Thread_Init* thread_init){
}

}

