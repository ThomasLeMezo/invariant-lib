#include "domain.h"

using namespace ibex;
using namespace std;
namespace invariant {

Domain::Domain(Graph *graph){
    m_graph = graph;
}

void Domain::contract_separator(Maze *maze, std::vector<Room*> &list_room_not_empty){
    if(m_sep_output != NULL){
        contract_separator(maze, m_graph->get_tree(), false, list_room_not_empty, true); // Output
    }
    else{
        m_graph->get_tree()->get_all_child_rooms_not_empty(list_room_not_empty, maze);
    }

    if(m_sep_input != NULL){
        contract_separator(maze, m_graph->get_tree(), false, list_room_not_empty, false); // Input
    }
    else{
        m_graph->get_tree()->get_all_child_rooms_not_empty(list_room_not_empty, maze);
    }
}

void Domain::contract_separator(Maze *maze, Pave_node *pave_node, bool all_out, std::vector<Room*> &list_pave_not_empty, bool output){
    if(all_out){
        if(pave_node->is_leaf()){
            Pave* p=pave_node->get_pave();
            for(Face * f:p->get_faces_vector()){
//                f->set_border(); // ?
                Door *d = f->get_doors()[maze];
                if(output)
                    d->set_empty_private_output();
                else
                    d->set_empty_private_input();
                d->synchronize();
            }
        }
        else{
            contract_separator(maze, pave_node->get_children().first, true, list_pave_not_empty, output);
            contract_separator(maze, pave_node->get_children().second, true, list_pave_not_empty, output);
        }
    }
    else if(pave_node->is_leaf()){
        Pave* p=pave_node->get_pave();
        bool not_empty = false;
        for(Face * f:p->get_faces_vector()){
            Door *d = f->get_doors()[maze];
            IntervalVector sep_in(p->get_dim());
            IntervalVector sep_out(p->get_dim());

            if(output){
                sep_in = d->get_output_private();
                sep_out = sep_in;
                m_sep_output->separate(sep_in, sep_out);
                d->set_output_private(sep_out & d->get_output_private());
            }
            else{
                sep_in = d->get_input_private();
                sep_out = sep_in;
                m_sep_input->separate(sep_in, sep_out);
                d->set_input_private(sep_out & d->get_input_private());
            }
            d->synchronize();
            if(!sep_out.is_empty())
                not_empty = true;
        }
        if(not_empty){
            list_pave_not_empty.push_back(p->get_rooms()[maze]);
        }
    }
    else{
        if(pave_node->get_emptyness()[maze]==true)
            return;

        IntervalVector position_in(pave_node->get_position());
        IntervalVector position_out(position_in);

        if(output)
            m_sep_output->separate(position_in, position_out);
        else
            m_sep_input->separate(position_in, position_out);

        if(position_out.is_empty()){
            // blue area only
            contract_separator(maze, pave_node->get_children().first, true, list_pave_not_empty, output);
            contract_separator(maze, pave_node->get_children().second, true, list_pave_not_empty, output);
        }
        else if(position_in.is_empty()){
            // yellow area only
            pave_node->get_all_child_rooms(list_pave_not_empty, maze);
        }
        else{
            // mix area
            contract_separator(maze,pave_node->get_children().first, false, list_pave_not_empty, output);
            contract_separator(maze,pave_node->get_children().second, false, list_pave_not_empty, output);
        }

    }
}

}
