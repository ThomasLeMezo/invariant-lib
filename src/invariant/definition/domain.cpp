#include "domain.h"

namespace invariant{
template<>
ppl::C_Polyhedron convert<ppl::C_Polyhedron>(const ibex::IntervalVector &iv){
    return iv_2_polyhedron(iv);
}

template<>
ibex::IntervalVector convert<ibex::IntervalVector>(const ibex::IntervalVector &iv){
    return iv;
}

template<>
ppl::C_Polyhedron convert<ppl::C_Polyhedron>(const ppl::C_Polyhedron &p){
    return p;
}

template<>
ibex::IntervalVector convert<ibex::IntervalVector>(const ppl::C_Polyhedron &p){
    return polyhedron_2_iv(p);
}

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
