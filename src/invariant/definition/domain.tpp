#include "domain.h"

namespace invariant {

template<typename _Tp>
Domain<_Tp>::Domain(SmartSubPaving<_Tp> *paving, DOMAIN_INITIALIZATION domain_init){
    m_subpaving = paving;
    m_domain_init = domain_init;
    omp_init_lock(&m_list_room_access);
    omp_init_lock(&m_lock_sep_output);
    omp_init_lock(&m_lock_sep_input);
}

template<typename _Tp>
Domain<_Tp>::~Domain(){
    omp_destroy_lock(&m_list_room_access);
    omp_destroy_lock(&m_lock_sep_output);
    omp_destroy_lock(&m_lock_sep_input);
}

template<typename _Tp>
void Domain<_Tp>::contract_domain(Maze<_Tp> *maze, std::vector<Room<_Tp>*> &list_room_deque){
    // ********** Initialize the maze ********** //
    // Global variables for all thread
    std::vector<Pave<_Tp>*> pave_border_list;
    m_subpaving->get_tree()->get_border_paves(pave_border_list);

#pragma omp parallel
    {
        Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_Tp>();
        #pragma omp barrier
#pragma omp for
        for(size_t i=0; i<m_subpaving->get_paves().size(); i++){
            Pave<_Tp> *p = m_subpaving->get_paves()[i];
            Room<_Tp> *r = p->get_rooms()[maze];
            if(!r->is_removed()){
                if(m_domain_init == FULL_DOOR)
                    r->set_full_private_with_father();
                else if(m_domain_init == FULL_WALL)
                    r->set_empty_private();
            }
        }

    // ********** Separator contraction ********** //
    // ==> ToDo : multithread !
        #pragma omp single
        {
            if(m_sep_output != nullptr){
                contract_separator(maze, m_subpaving->get_tree(), true, SEP_UNKNOWN); // Output
            }
            if(m_sep_input != nullptr){
                contract_separator(maze, m_subpaving->get_tree(), false, SEP_UNKNOWN); // Input
            }
        }

        // ********** Border contraction ********** //
        if(!(m_subpaving->size()==1 && m_domain_init == FULL_DOOR))
            contract_border(maze, pave_border_list); // (not in single because of omp for inside)

        // ********** Intersection/Union contraction with other mazes ********** //
        // => To proceed after initial set
        #pragma omp single
        {
            contract_inter_maze(maze);
            contract_union_maze(maze);
        }

        // Sychronization of rooms
        #pragma omp for
        for(size_t i=0; i<m_subpaving->get_paves().size(); i++){
            Pave<_Tp> *p = m_subpaving->get_paves()[i];
            Room<_Tp> *r = p->get_rooms()[maze];
            r->synchronize();
        }
        delete_thread_init<_Tp>(thread_init);
    }

    // ********** Add additional rooms to deque ********** //
    if(m_domain_init == FULL_DOOR){
        m_subpaving->get_tree()->get_all_child_rooms_not_empty(list_room_deque, maze);
    }
    if(m_domain_init == FULL_WALL && m_subpaving->get_paves().size()>1){
        // When initial condition is not link with active paves, need to add all paves (case removed full pave in bracketing)
        //        list_room_deque.insert(list_room_deque.end(), get_initial_room_list().begin(), get_initial_room_list().end());
        m_subpaving->get_tree()->get_all_child_rooms(list_room_deque, maze);
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_separator(Maze<_Tp> *maze, Pave_node<_Tp> *pave_node, bool output, DOMAIN_SEP accelerator){
    //    ibex::IntervalVector test(2);
    //    test[0] = ibex::Interval(0, 2);
    //    test[1] = ibex::Interval(-3.5, -1.125);
    //    if(test==pave_node->get_position())
    //        std::cout << "Test" << std::endl;

    if(pave_node->get_removed()[maze])
        return;
    switch (accelerator) {
    case SEP_INSIDE:{
        if(!pave_node->get_fullness()[maze] || m_domain_init==FULL_WALL){
            if(pave_node->is_leaf()){
                Pave<_Tp>* p = pave_node->get_pave();
                Room<_Tp> *r = p->get_rooms()[maze];
                if(!r->is_removed()){
                    if(output){
                        if(m_domain_init == FULL_WALL)
                            r->set_full_initial_door_output();
                        else
                            r->set_full_private_output();
                    }
                    else{
                        if(m_domain_init == FULL_WALL)
                            r->set_full_initial_door_input();
                        else
                            r->set_full_private_input();
                    }
                }
            }
            else{
#pragma omp task
                contract_separator(maze, pave_node->get_children().first, output, SEP_INSIDE);
#pragma omp task
                contract_separator(maze, pave_node->get_children().second, output, SEP_INSIDE);
#pragma omp taskwait
            }
        }
    }
        break;
    case SEP_OUTSIDE:{
        if(!pave_node->get_emptyness()[maze] || m_domain_init==FULL_DOOR){
            if(pave_node->is_leaf()){
                Pave<_Tp>* p = pave_node->get_pave();
                Room<_Tp> *r = p->get_rooms()[maze];
                if(!r->is_removed()){
                    if(output)
                        r->set_empty_private_output();
                    else
                        r->set_empty_private_input();
                }
            }
            else{
                if(m_domain_init != FULL_WALL){
#pragma omp task
                    contract_separator(maze, pave_node->get_children().first, output, SEP_OUTSIDE);
#pragma omp task
                    contract_separator(maze, pave_node->get_children().second, output, SEP_OUTSIDE);
#pragma omp taskwait
                }
            }
        }
    }
        break;
    case SEP_UNKNOWN:{
        ibex::IntervalVector x_in(pave_node->get_position());
        ibex::IntervalVector x_out(x_in);
        if(output){
            omp_set_lock(&m_lock_sep_output);
            m_sep_output->separate(x_in, x_out);
            omp_unset_lock(&m_lock_sep_output);
        }
        else{
            omp_set_lock(&m_lock_sep_input);
            m_sep_input->separate(x_in, x_out);
            omp_unset_lock(&m_lock_sep_input);
        }

        if(pave_node->is_leaf()){
            Pave<_Tp> *p = pave_node->get_pave();
            Room<_Tp> *r = p->get_rooms()[maze];
            if(!r->is_removed()){
                if(x_in.is_empty()){ // Inside the constraint
                    if(output){
                        if(m_domain_init == FULL_WALL)
                            r->set_full_initial_door_output();
                        else
                            r->set_full_private_output();
                    }
                    else{
                        if(m_domain_init == FULL_WALL)
                            r->set_full_initial_door_input();
                        else
                            r->set_full_private_input();
                    }
                }
                else if(x_out.is_empty()){  // Outside the constraint
                    if(output)
                        r->set_empty_private_output();
                    else
                        r->set_empty_private_input();
                }
                else{ // Inside & Outside the constraint => x_out not empty & x_in not empty
                    if(output){
                        if(m_domain_init == FULL_WALL){
                            r->set_initial_door_output(convert<_Tp>(x_out)); // r->set_full_initial_door_output();
                        }
                        else{
                            contract_box(r, x_out, DOOR_INPUT); // r->set_full_private_output();
                        }
                    }
                    else{
                        if(m_domain_init == FULL_WALL){
                            r->set_initial_door_input(convert<_Tp>(x_out)); // r->set_full_initial_door_input();
                        }
                        else{
                            contract_box(r, x_out, DOOR_OUTPUT); // r->set_full_private_input();
                        }
                    }
                }
            }
        }
        else{
            // Determine the accelerator
            if(x_in.is_empty()){
                // Completly inside the constraint
#pragma omp task
                contract_separator(maze, pave_node->get_children().first, output, SEP_INSIDE);
#pragma omp task
                contract_separator(maze, pave_node->get_children().second, output, SEP_INSIDE);
#pragma taskwait
            }
            else if(x_out.is_empty()){
                // Completly outside the constraint
#pragma omp task
                contract_separator(maze, pave_node->get_children().first, output, SEP_OUTSIDE);
#pragma omp task
                contract_separator(maze, pave_node->get_children().second, output, SEP_OUTSIDE);
#pragma omp taskwait
            }
            else{
                // Mix area (outside & inside)
#pragma omp task
                contract_separator(maze,pave_node->get_children().first, output, SEP_UNKNOWN);
#pragma omp task
                contract_separator(maze,pave_node->get_children().second, output, SEP_UNKNOWN);
#pragma omp taskwait
            }
        }
    }
        break;
    default:
        break;
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_border(Maze<_Tp> *maze, std::vector<Pave<_Tp>*> &pave_border_list){
#pragma omp for
    for(size_t i=0; i<pave_border_list.size(); i++){
        Pave<_Tp> *p = pave_border_list[i];
        Room<_Tp> *r = p->get_rooms()[maze];
        if(!r->is_removed()){
            for(Face<_Tp> *f:p->get_faces_vector()){
                if(f->is_border()){
                    Door<_Tp> *d = f->get_doors()[maze];
                    if(m_border_path_in)
                        d->set_full_private_input();
                    else{
                        if(m_domain_init != FULL_WALL && !r->get_contain_zero())
                            d->set_empty_private_input();
                    }

                    if(m_border_path_out)
                        d->set_full_private_output();
                    else{
                        if(m_domain_init != FULL_WALL && !r->get_contain_zero())
                            d->set_empty_private_output();
                    }
                }
            }
        }
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_inter_maze(Maze<_Tp> *maze){
    if(m_maze_list_inter.empty())
        return;
    std::cout << " ==> contract inter maze" << std::endl;

    /// ************************************ FULL_DOOR CASE ************************************ ///
    if(maze->get_domain()->get_init()==FULL_DOOR && !maze->is_escape_trajectories()){
        std::vector<Room<_Tp> *> room_list;
        m_subpaving->get_tree()->get_all_child_rooms_not_empty(room_list, maze);
        for(Maze<_Tp> *maze_inter:m_maze_list_inter){
            if(!maze_inter->is_escape_trajectories()){
#pragma omp for
                for(size_t i=0; i<room_list.size(); i++){
                    Room<_Tp> *r = room_list[i];
                    Pave<_Tp> *p = r->get_pave();
                    Room<_Tp> *r_inter = p->get_rooms()[maze_inter];
                    *r &= *r_inter;
                    // Add a contraction of the initial condition !
                }
            }
        }
    }
    /// ************************************ FULL_WALL CASE ************************************ ///
    else if(maze->get_domain()->get_init()==FULL_WALL){
        std::vector<Room<_Tp> *> room_list_initial = maze->get_initial_room_list();
        // Contract initial condition
        //        for(Maze<_Tp> *maze_inter:m_maze_list_inter){
        //            if(maze_inter->get_contract_once()){
        //#pragma omp parallel
        //                {
        //                    Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_Tp>();
        //#pragma omp for
        //                    for(size_t i=0; i<room_list_initial.size(); i++){
        //                        Room<_Tp> *r = room_list_initial[i];
        //                        Pave<_Tp> *p = r->get_pave();
        //                        Room<_Tp> *r_inter = p->get_rooms()[maze_inter];
        //                        if(r->is_initial_door_input())
        //                            r->set_initial_door_input(r->get_initial_door_input() & r_inter->get_hull_typed());
        //                        if(r->is_initial_door_output())
        //                            r->set_initial_door_output(r->get_initial_door_output() & r_inter->get_hull_typed());
        //                        // Add a contraction of the initial condition !
        //                        r->synchronize();
        //                    }
        //                    delete_thread_init<_Tp>(thread_init);
        //                }
        //            }
        //        }

        // Contract father_hull
        std::vector<Pave<_Tp> *> room_list = maze->get_subpaving()->get_paves();
        for(Maze<_Tp> *maze_inter:m_maze_list_inter){
#pragma omp for
            for(size_t i=0; i<room_list.size(); i++){
                Room<_Tp> *r = room_list[i]->get_rooms()[maze];
                if(!r->is_removed() && r->is_father_hull()){
                    Pave<_Tp> *p = r->get_pave();
                    Room<_Tp> *r_inter = p->get_rooms()[maze_inter];
                    if(maze_inter->get_contract_once()){
                        r->set_father_hull(r->get_father_hull() & r_inter->get_hull_typed());
                    }
                    else{
                        if(r_inter->is_father_hull())
                            r->set_father_hull(r->get_father_hull() & r_inter->get_father_hull());
                    }
                }
            }
        }
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_union_maze(Maze<_Tp> *maze){
    if(m_maze_list_union.empty())
        return;
    std::vector<Room<_Tp> *> room_list;
    m_subpaving->get_tree()->get_all_child_rooms_not_full(room_list, maze);

    for(Maze<_Tp> *maze_union:m_maze_list_union){
#pragma omp for
        for(size_t i=0; i<room_list.size(); i++){
            Room<_Tp> *r = room_list[i];
            Pave<_Tp> *p = r->get_pave();
            Room<_Tp> *r_inter = p->get_rooms()[maze_union];
            *r |= *r_inter;
        }
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_box(Room<_Tp> *room, const ibex::IntervalVector& initial_condition, DOOR_SELECTOR doorSelector){
    // Warning : no inversion of constraint (output=>input) because this is not a propagation as in the case of FULL_WALL
    for(Face<_Tp> *f:room->get_pave()->get_faces_vector()){
        Door<_Tp> *d = f->get_doors()[room->get_maze()];
        if(doorSelector == DOOR_OUTPUT || doorSelector == DOOR_INPUT_OUTPUT)
            d->set_output_private(convert<_Tp>(initial_condition & convert<ibex::IntervalVector>(d->get_output_private())));
        if(doorSelector == DOOR_INPUT || doorSelector == DOOR_INPUT_OUTPUT)
            d->set_input_private(convert<_Tp>(initial_condition & convert<ibex::IntervalVector>(d->get_input_private())));
    }
}

}
