#include "domain.h"
#include "door.h"

using namespace ibex;
using namespace std;
namespace invariant {

Domain::Domain(Graph *graph){
    m_graph = graph;
}

void Domain::contract_domain(Maze *maze, std::vector<Room*> &list_room_deque){
    if(m_sep_output != NULL){
        contract_separator(maze, m_graph->get_tree(), list_room_deque, true, SEP_UNKNOWN); // Output
    }
    else{
        if(maze->get_type() == MAZE_CONTRACTOR)
            m_graph->get_tree()->get_all_child_rooms_not_empty(list_room_deque, maze);
        else
            m_graph->get_tree()->get_all_child_rooms_not_full(list_room_deque, maze);
    }

    if(m_sep_input != NULL){
        contract_separator(maze, m_graph->get_tree(), list_room_deque, false, SEP_UNKNOWN); // Input
    }
    else{
        if(maze->get_type() == MAZE_CONTRACTOR)
            m_graph->get_tree()->get_all_child_rooms_not_empty(list_room_deque, maze);
        else
            m_graph->get_tree()->get_all_child_rooms_not_full(list_room_deque, maze);
    }

    contract_border(maze, list_room_deque);
}

void Domain::contract_separator(Maze *maze, Pave_node *pave_node, std::vector<Room*> &list_pave_deque, bool output, DOMAIN_SEP accelerator){
    MazeType type = maze->get_type();
    switch (accelerator) {
    case SEP_INSIDE:{
        if(!pave_node->get_fullness()[maze]){
            if(pave_node->is_leaf()){
                Pave* p = pave_node->get_pave();
                Room *r = p->get_rooms()[maze];
                if(output)
                    r->set_full_private_output();
                else
                    r->set_full_private_input();
                if(type == MAZE_PROPAGATOR)
                    p->get_neighbors_room(maze, list_pave_deque);
                r->synchronize();
            }
            else{
                contract_separator(maze, pave_node->get_children().first, list_pave_deque, output, SEP_INSIDE);
                contract_separator(maze, pave_node->get_children().second, list_pave_deque, output, SEP_INSIDE);
            }
        }
    }
        break;
    case SEP_OUTSIDE:{
        if(!pave_node->get_emptyness()[maze]){
            if(pave_node->is_leaf()){
                Pave* p = pave_node->get_pave();
                Room *r = p->get_rooms()[maze];
                if(output)
                    r->set_empty_private_output();
                else
                    r->set_empty_private_input();
                if(type == MAZE_CONTRACTOR)
                    p->get_neighbors_room(maze, list_pave_deque);
                r->synchronize();
            }
            else{
                contract_separator(maze, pave_node->get_children().first, list_pave_deque, output, SEP_OUTSIDE);
                contract_separator(maze, pave_node->get_children().second, list_pave_deque, output, SEP_OUTSIDE);
            }
        }
    }
        break;
    case SEP_UNKNOWN:{
        IntervalVector position_in(pave_node->get_position());
        IntervalVector position_out(position_in);
        if(output)
            m_sep_output->separate(position_in, position_out);
        else
            m_sep_input->separate(position_in, position_out);

        if(pave_node->is_leaf()){
            Pave* p = pave_node->get_pave();
            if(position_in.is_empty()){
                // Inside constraint
                if(output)
                    p->get_rooms()[maze]->set_full_private_output();
                else
                    p->get_rooms()[maze]->set_full_private_input();
                if(type == MAZE_PROPAGATOR)
                    list_pave_deque.push_back(p->get_rooms()[maze]);
            }
            else if(position_out.is_empty()){
                // Outside constraint
                if(output)
                    p->get_rooms()[maze]->set_empty_private_output();
                else
                    p->get_rooms()[maze]->set_empty_private_input();
                if(type == MAZE_CONTRACTOR){
                    list_pave_deque.push_back(p->get_rooms()[maze]);
                    p->get_neighbors_room(maze, list_pave_deque);
                }
            }
            else{
                // Mix => all full (over approximation)
                if(output)
                    p->get_rooms()[maze]->set_full_private_output();
                else
                    p->get_rooms()[maze]->set_full_private_input();
                if(type == MAZE_PROPAGATOR)
                    list_pave_deque.push_back(p->get_rooms()[maze]);
            }
        }
        else{
            // Determine the accelerator
            if(position_in.is_empty()){
                // Completly inside the constraint
                contract_separator(maze, pave_node->get_children().first, list_pave_deque, output, SEP_INSIDE);
                contract_separator(maze, pave_node->get_children().second, list_pave_deque, output, SEP_INSIDE);
            }
            else if(position_out.is_empty()){
                // Completly outside the constraint
                contract_separator(maze, pave_node->get_children().first, list_pave_deque, output, SEP_OUTSIDE);
                contract_separator(maze, pave_node->get_children().second, list_pave_deque, output, SEP_OUTSIDE);
            }
            else{
                // Mix area (outside & inside)
                contract_separator(maze,pave_node->get_children().first, list_pave_deque, output, SEP_UNKNOWN);
                contract_separator(maze,pave_node->get_children().second, list_pave_deque, output, SEP_UNKNOWN);
            }
        }
    }
        break;
    default:
        break;
    }
}

void Domain::contract_border(Maze *maze, std::vector<Room*> &list_room_deque){
    vector<Pave*> pave_border_list;
    m_graph->get_tree()->get_border_paves(pave_border_list);
    MazeType type = maze->get_type();

    for(Pave *p:pave_border_list){
        for(Face *f:p->get_faces_vector()){
            if(f->is_border()){
                Door *d = f->get_doors()[maze];
                if(m_border_path_in)
                    d->set_full_private_input();
                else
                    d->set_empty_private_input();

                if(m_border_path_out)
                    d->set_full_private_output();
                else
                    d->set_empty_private_output();
                d->synchronize();
            }
        }
        if(type == MAZE_PROPAGATOR && (m_border_path_in || m_border_path_out)){
            if(!p->get_rooms()[maze]->is_full())
                list_room_deque.push_back(p->get_rooms()[maze]);
        }
        if(type == MAZE_CONTRACTOR && (!m_border_path_in || !m_border_path_out)){
            if(!p->get_rooms()[maze]->is_empty())
                list_room_deque.push_back(p->get_rooms()[maze]);
        }
    }
}

}
