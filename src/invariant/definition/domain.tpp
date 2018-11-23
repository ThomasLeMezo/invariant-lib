#include "domain.h"

namespace invariant {

template<typename _Tp>
Domain<_Tp>::Domain(SmartSubPaving<_Tp> *paving, DOMAIN_INITIALIZATION domain_init){
  m_subpaving = paving;
  m_domain_init = domain_init;
  omp_init_lock(&m_list_room_access);
  omp_init_lock(&m_lock_sep);
  omp_init_lock(&m_lock_sep_zero);
}

template<typename _Tp>
Domain<_Tp>::~Domain(){
  omp_destroy_lock(&m_list_room_access);
  omp_destroy_lock(&m_lock_sep);
  omp_destroy_lock(&m_lock_sep_zero);
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
        if(m_domain_init == FULL_WALL)
          r->set_empty_private();
        r->reset_first_contract();
      }
    }

    // ********** Separator contraction ********** //
    // ==> ToDo : multithread ! => not ready because of set_initial_door_output/input test
#pragma omp single
    {
      if(m_sep_output != nullptr){
        contract_separator(maze, m_subpaving->get_tree(), true, SEP_UNKNOWN, m_sep_output); // Output
      }
      if(m_sep_input != nullptr){
        contract_separator(maze, m_subpaving->get_tree(), false, SEP_UNKNOWN, m_sep_input); // Input
      }
    }

    // ********** Border contraction ********** //
    if(!(m_subpaving->size()==1 && m_domain_init == FULL_DOOR))
      contract_border(maze, pave_border_list); // (not in single because of omp for inside)

    // ********** Intersection/Union contraction with other mazes ********** //
    // => To proceed after initial set (modify initial door)
#pragma omp single
    {
      contract_inter_maze(maze);
      contract_union_maze(maze);
      contract_initialization_inter_maze(maze);
      contract_initialization_union_maze(maze);
    }

    // Sychronization of rooms
#pragma omp for
    for(size_t i=0; i<m_subpaving->get_paves().size(); i++){
      Pave<_Tp> *p = m_subpaving->get_paves()[i];
      Room<_Tp> *r = p->get_rooms()[maze];
      if(!r->is_removed()){
        r->synchronize();
      }
    }
    delete_thread_init<_Tp>(thread_init);
  }

  // ********** Add additional rooms to deque ********** //
  if(m_domain_init == FULL_DOOR){
    m_subpaving->get_tree()->get_all_child_rooms(list_room_deque, maze); // ToDo: correction of not empty function
  }
  if(m_domain_init == FULL_WALL && m_subpaving->get_paves().size()>1){
    // When initial condition is not link with active paves, need to add all paves (case removed full pave in bracketing)
    //        list_room_deque.insert(list_room_deque.end(), get_initial_room_list().begin(), get_initial_room_list().end());
    m_subpaving->get_tree()->get_all_child_rooms(list_room_deque, maze);
  }
}

template<typename _Tp>
void Domain<_Tp>::contract_separator(Maze<_Tp> *maze, Pave_node<_Tp> *pave_node, bool output, DOMAIN_SEP accelerator, ibex::Sep* sep){
  //    ibex::IntervalVector test(2);
  //    test[0] = ibex::Interval(0, 2);
  //    test[1] = ibex::Interval(-3.5, -1.125);
  //    if(test==pave_node->get_position())
  //        std::cout << "Test" << std::endl;

  if(pave_node->get_removed()[maze])
    return;
  switch (accelerator) {
  case SEP_INSIDE:
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
        contract_separator(maze, pave_node->get_children().first, output, SEP_INSIDE, sep);
#pragma omp task
        contract_separator(maze, pave_node->get_children().second, output, SEP_INSIDE, sep);
#pragma omp taskwait
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
          contract_separator(maze, pave_node->get_children().first, output, SEP_OUTSIDE, sep);
#pragma omp task
          contract_separator(maze, pave_node->get_children().second, output, SEP_OUTSIDE, sep);
#pragma omp taskwait
        }
      }
    }
  }
    break;
  case SEP_UNKNOWN:{
    ibex::IntervalVector x_in(pave_node->get_position());
    ibex::IntervalVector x_out(x_in);

    omp_set_lock(&m_lock_sep);
    sep->separate(x_in, x_out);
    omp_unset_lock(&m_lock_sep);

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
        contract_separator(maze, pave_node->get_children().first, output, SEP_INSIDE, sep);
#pragma omp task
        contract_separator(maze, pave_node->get_children().second, output, SEP_INSIDE, sep);
#pragma taskwait
      }
      else if(x_out.is_empty()){
        // Completly outside the constraint
#pragma omp task
        contract_separator(maze, pave_node->get_children().first, output, SEP_OUTSIDE, sep);
#pragma omp task
        contract_separator(maze, pave_node->get_children().second, output, SEP_OUTSIDE, sep);
#pragma omp taskwait
      }
      else{
        // Mix area (outside & inside)
#pragma omp task
        contract_separator(maze,pave_node->get_children().first, output, SEP_UNKNOWN, sep);
#pragma omp task
        contract_separator(maze,pave_node->get_children().second, output, SEP_UNKNOWN, sep);
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
bool Domain<_Tp>::contract_zero_door(Room<_Tp> *r){
  if(m_sep_zero == nullptr)
    return false;
  if(!r->is_removed()){
    ibex::IntervalVector x_in(r->get_pave()->get_position());
    ibex::IntervalVector x_out(x_in);

    omp_set_lock(&m_lock_sep);
    m_sep_zero->separate(x_in, x_out);
    omp_unset_lock(&m_lock_sep);


    if(x_in.is_empty()){ // Inside the constraint
      return false;
    }
    else if(x_out.is_empty()){  // Outside the constraint
      if(m_domain_init == FULL_WALL)
        r->set_empty_private();
      if(m_domain_init == FULL_DOOR)
        r->set_full_private();
      return false;
    }
    else{ // Inside & Outside the constraint => x_out not empty & x_in not empty
      if(m_domain_init == FULL_DOOR)
        propagate_box(r, x_out, DOOR_INPUT_OUTPUT); // r->set_full_initial_door_output();
      if(m_domain_init == FULL_WALL)
        contract_box(r, x_out, DOOR_INPUT_OUTPUT); // r->set_full_private_output();
      return true;
    }
  }
  else
    return false;
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
  if(m_maze_list_inter_initial_condition.empty() && m_maze_list_inter_father_hull.empty())
    return;
  std::cout << " ==> contract inter maze" << std::endl;

  /// ************************************ FULL_DOOR CASE ************************************ ///
  if(maze->get_domain()->get_init()==FULL_DOOR && !maze->is_escape_trajectories()){
    std::vector<Room<_Tp> *> room_list;
    m_subpaving->get_tree()->get_all_child_rooms_not_empty_private(room_list, maze);
    for(Maze<_Tp> *maze_inter:m_maze_list_inter_father_hull){
      if(!maze_inter->is_escape_trajectories()){
        //#pragma omp for
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
    // Contract the initial condition (input & output)

    for(Maze<_Tp> *maze_inter:m_maze_list_inter_initial_condition){
      if(maze_inter->get_contract_once()){
        for(size_t i=0; i<room_list_initial.size(); i++){
          Room<_Tp> *r = room_list_initial[i];
          Pave<_Tp> *p = r->get_pave();
          Room<_Tp> *r_inter = p->get_rooms()[maze_inter];

          _Tp hull = r_inter->get_hull_typed();
          if(r->is_initial_door_input()){
            _Tp init_door_input = r->get_initial_door_input() & hull;
            if(init_door_input.is_empty()){
              r->reset_init_door_input();
            }
            else
              r->set_initial_door_input(init_door_input);
          }
          if(r->is_initial_door_output()){
            _Tp init_door_output = r->get_initial_door_output() & hull;
            if(init_door_output.is_empty()){
              r->reset_init_door_output();
            }
            else
              r->set_initial_door_output(init_door_output);
          }
        }
      }
    }
  }

  // Contract father_hull (usefull to limit propagation)
  std::vector<Pave<_Tp> *> room_list = maze->get_subpaving()->get_paves();
  for(Maze<_Tp> *maze_inter:m_maze_list_inter_father_hull){
    //#pragma omp for
    for(size_t i=0; i<room_list.size(); i++){
      Room<_Tp> *r = room_list[i]->get_rooms()[maze];
      if(!r->is_removed() && r->is_father_hull()){
        Pave<_Tp> *p = r->get_pave();
        Room<_Tp> *r_inter = p->get_rooms()[maze_inter];

        if(!r_inter->is_initial_door_input() && !r_inter->is_initial_door_output()){ // Cause a bug if no condition
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

  if(maze->get_domain()->get_init()==FULL_DOOR){
    std::vector<Room<_Tp> *> room_list;
    m_subpaving->get_tree()->get_all_child_rooms_not_empty_private(room_list, maze);
    //        m_subpaving->get_tree()->get_all_child_rooms(room_list, maze);
    for(Maze<_Tp> *maze_union:m_maze_list_union){
      if(maze_union->get_contract_once()){
        for(size_t i=0; i<room_list.size(); i++){
          Room<_Tp> *r = room_list[i];
          Pave<_Tp> *p = r->get_pave();
          Room<_Tp> *r_union = p->get_rooms()[maze_union];
          _Tp hull = r_union->get_hull_typed();
          if(!hull.is_empty())
            r->set_union_hull(r_union->get_hull_typed());
        }
      }
    }
  }
  else{
    std::vector<Room<_Tp> *> room_list_initial = maze->get_initial_room_list();
    // Contract the initial condition (input & output)
    // BUG ???????????? initialy: m_maze_list_inter_initial_condition
    // (m_maze_list_union) or m_maze_list_inter_initial_condition ?
    for(Maze<_Tp> *maze_union:m_maze_list_inter_initial_condition){
      if(maze_union->get_contract_once()){
        for(size_t i=0; i<room_list_initial.size(); i++){
          Room<_Tp> *r = room_list_initial[i];
          Pave<_Tp> *p = r->get_pave();
          Room<_Tp> *r_union = p->get_rooms()[maze_union];
          if(r->is_initial_door_input())
            r->set_initial_door_input(r->get_initial_door_input() | r_union->get_hull_typed());
          if(r->is_initial_door_output())
            r->set_initial_door_output(r->get_initial_door_output() | r_union->get_hull_typed());
        }
      }
    }
  }
}

// Note: same function as contract_union_maze ?
template<typename _Tp>
void Domain<_Tp>::contract_initialization_union_maze(Maze<_Tp> *maze){
  if(m_maze_list_initialization_union.empty())
    return;

  std::vector<Room<_Tp> *> room_list;
  m_subpaving->get_tree()->get_all_child_rooms_not_empty_private(room_list, maze);

  for(Maze<_Tp> *maze_union:m_maze_list_initialization_union){
    if(maze_union->get_contract_once()){
      for(size_t i=0; i<room_list.size(); i++){
        Room<_Tp> *r = room_list[i];
        Pave<_Tp> *p = r->get_pave();
        Room<_Tp> *r_union = p->get_rooms()[maze_union];
        *r |= *r_union;
      }
    }
  }
}

template<typename _Tp>
void Domain<_Tp>::contract_initialization_inter_maze(Maze<_Tp> *maze){
  if(m_maze_list_initialization_inter.empty())
    return;

  std::vector<Room<_Tp> *> room_list;
  m_subpaving->get_tree()->get_all_child_rooms(room_list, maze);
  for(Maze<_Tp> *maze_union:m_maze_list_initialization_inter){
    if(maze_union->get_contract_once()){
      for(size_t i=0; i<room_list.size(); i++){
        Room<_Tp> *r = room_list[i];
        Pave<_Tp> *p = r->get_pave();
        Room<_Tp> *r_union = p->get_rooms()[maze_union];
        *r &= *r_union;
      }
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

template<typename _Tp>
void Domain<_Tp>::propagate_box(Room<_Tp> *room, const ibex::IntervalVector& initial_condition, DOOR_SELECTOR doorSelector){
  // Warning : no inversion of constraint (output=>input) because this is not a propagation as in the case of FULL_WALL
  for(Face<_Tp> *f:room->get_pave()->get_faces_vector()){
    Door<_Tp> *d = f->get_doors()[room->get_maze()];
    if(doorSelector == DOOR_OUTPUT || doorSelector == DOOR_INPUT_OUTPUT)
      d->set_output_private(convert<_Tp>(convert<ibex::IntervalVector>(d->get_output_private()) | (initial_condition & convert<ibex::IntervalVector>(d->get_face()->get_position_typed()))));
    if(doorSelector == DOOR_INPUT || doorSelector == DOOR_INPUT_OUTPUT)
      d->set_input_private(convert<_Tp>(convert<ibex::IntervalVector>(d->get_output_private()) | (initial_condition & convert<ibex::IntervalVector>(d->get_face()->get_position_typed()))));
  }
}

}
