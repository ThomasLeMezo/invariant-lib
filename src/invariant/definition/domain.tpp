#include "domain.h"

namespace invariant {

template<typename _Tp>
Domain<_Tp>::Domain(SmartSubPaving<_Tp> *paving, DOMAIN_INITIALIZATION domain_init){
    m_subpaving = paving;
    m_domain_init = domain_init;
}

template<typename _Tp>
void Domain<_Tp>::contract_domain(Maze<_Tp> *maze, std::vector<Room<_Tp>*> &list_room_deque){
    // ********** Initialize the maze ********** //
#pragma omp parallel
    {
        Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_Tp>();
#pragma omp for
        for(size_t i=0; i<m_subpaving->get_paves().size(); i++){
            Pave<_Tp> *p = m_subpaving->get_paves()[i];
            Room<_Tp> *r = p->get_rooms()[maze];
            if(!r->is_removed()){
                if(m_domain_init == FULL_DOOR)
                    r->set_full_private_with_father();
                else if(m_domain_init == FULL_WALL)
                    r->set_empty_private();
                r->synchronize();
                //            r->reset_update_neighbors();
            }
        }
        delete_thread_init<_Tp>(thread_init);
    }

    // ********** Separator contraction ********** //
    // ==> ToDo : multithread !
    if(m_sep_output != nullptr)
        contract_separator(maze, m_subpaving->get_tree(), list_room_deque, true, SEP_UNKNOWN); // Output
    if(m_sep_input != nullptr)
        contract_separator(maze, m_subpaving->get_tree(), list_room_deque, false, SEP_UNKNOWN); // Input

    // ********** Border contraction ********** //
    contract_border(maze, list_room_deque);

    // ********** Intersection/Union contraction with other mazes ********** //
    // => Proceed after initial set
    contract_inter_maze(maze);
    contract_union_maze(maze);

    // ********** Add additional rooms to deque ********** //
    if(m_domain_init == FULL_DOOR){
        m_subpaving->get_tree()->get_all_child_rooms_not_empty(list_room_deque, maze);
    }
    if(m_domain_init == FULL_WALL && m_subpaving->get_paves().size()>1){
        // When initial condition is not link with active paves ?
        // (OK ?) Wrong function -> need to add neighbours of full paves instead of not_empty
        // TODO : correction of this method ?
        //        m_subpaving->get_tree()->get_all_child_rooms_not_empty(list_room_deque, maze); // Wrong ?
        m_subpaving->get_tree()->get_all_child_rooms(list_room_deque, maze); // ??
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_separator(Maze<_Tp> *maze, Pave_node<_Tp> *pave_node, std::vector<Room<_Tp>*> &list_room_deque, bool output, DOMAIN_SEP accelerator){
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
                    if(m_domain_init == FULL_WALL){
                        list_room_deque.push_back(r);
                    }
                    r->synchronize();
                }
            }
            else{
                contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_INSIDE);
                contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_INSIDE);
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
                    r->synchronize();
                }
            }
            else{
                if(m_domain_init != FULL_WALL){
                    contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_OUTSIDE);
                    contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_OUTSIDE);
                }
            }
        }
    }
        break;
    case SEP_UNKNOWN:{
        ibex::IntervalVector x_in(pave_node->get_position());
        ibex::IntervalVector x_out(x_in);
        if(output)
            m_sep_output->separate(x_in, x_out);
        else
            m_sep_input->separate(x_in, x_out);

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
                    if(m_domain_init == FULL_WALL){
                        list_room_deque.push_back(r);
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

                    if(m_domain_init == FULL_WALL){
                        list_room_deque.push_back(r);
                    }
                }
                r->synchronize();
            }
        }
        else{
            // Determine the accelerator
            if(x_in.is_empty()){
                // Completly inside the constraint
                contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_INSIDE);
                contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_INSIDE);
            }
            else if(x_out.is_empty()){
                // Completly outside the constraint
                contract_separator(maze, pave_node->get_children().first, list_room_deque, output, SEP_OUTSIDE);
                contract_separator(maze, pave_node->get_children().second, list_room_deque, output, SEP_OUTSIDE);
            }
            else{
                // Mix area (outside & inside)
                contract_separator(maze,pave_node->get_children().first, list_room_deque, output, SEP_UNKNOWN);
                contract_separator(maze,pave_node->get_children().second, list_room_deque, output, SEP_UNKNOWN);
            }
        }
    }
        break;
    default:
        break;
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_border(Maze<_Tp> *maze, std::vector<Room<_Tp>*> &list_room_deque){
    if(m_subpaving->size()==1 && m_domain_init == FULL_DOOR)
        return;

    std::vector<Pave<_Tp>*> pave_border_list;
    m_subpaving->get_tree()->get_border_paves(pave_border_list);

#pragma omp parallel
    {
        Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_Tp>();
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
                        d->synchronize();
                    }
                }

                if(m_domain_init == FULL_WALL && (m_border_path_in || m_border_path_out)){
                    if(!p->get_rooms()[maze]->is_full()){
#pragma omp critical(dqueue)
                        {
                            list_room_deque.push_back(p->get_rooms()[maze]);
                        }
                    }
                }
                if(m_domain_init == FULL_DOOR && (!m_border_path_in || !m_border_path_out)){
                    if(!p->get_rooms()[maze]->is_empty()){
#pragma omp critical(dqueue)
                        {
                            list_room_deque.push_back(p->get_rooms()[maze]);
                        }
                    }
                }
            }
        }
        delete_thread_init<_Tp>(thread_init);
    }
}

template<typename _Tp>
void Domain<_Tp>::contract_inter_maze(Maze<_Tp> *maze){
    if(m_maze_list_inter.empty() || maze->is_escape_trajectories())
        return;
    std::vector<Room<_Tp> *> room_list;
    m_subpaving->get_tree()->get_all_child_rooms_not_empty(room_list, maze);

    for(Maze<_Tp> *maze_inter:m_maze_list_inter){
        if(!maze_inter->is_escape_trajectories()){
#pragma omp parallel
            {
                Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_Tp>();
#pragma omp for
                for(size_t i=0; i<room_list.size(); i++){
                    Room<_Tp> *r = room_list[i];
                    Pave<_Tp> *p = r->get_pave();
                    Room<_Tp> *r_inter = p->get_rooms()[maze_inter];
                    *r &= *r_inter;
                    r->synchronize();
                }
                delete_thread_init<_Tp>(thread_init);
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
#pragma omp parallel
        {
            Parma_Polyhedra_Library::Thread_Init* thread_init = initialize_thread<_Tp>();
#pragma omp for
            for(size_t i=0; i<room_list.size(); i++){
                Room<_Tp> *r = room_list[i];
                Pave<_Tp> *p = r->get_pave();
                Room<_Tp> *r_inter = p->get_rooms()[maze_union];
                *r |= *r_inter;
                r->synchronize();
            }
            delete_thread_init<_Tp>(thread_init);
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
