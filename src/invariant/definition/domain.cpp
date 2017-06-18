#include "domain.h"

using namespace ibex;
using namespace std;
namespace invariant {

Domain::Domain(Graph *graph){
    m_graph = graph;
}

void Domain::contract_separator(Maze *maze, std::vector<Room*> &list_pave_not_empty){
    if(m_sep_output != NULL)
        contract_separator(maze, m_graph->get_pave_node(), false, list_pave_not_empty, true); // Output
    if(m_sep_input != NULL)
        contract_separator(maze, m_graph->get_pave_node(), false, list_pave_not_empty, false); // Input
}

void Domain::contract_separator(Maze *maze, Pave_node *pave_node, bool all_out, std::vector<Room*> &list_pave_not_empty, bool output){
    if(all_out){
        if(pave_node->is_leaf()){
            Pave* p=pave_node->get_pave();
            for(Face * f:p->get_faces_vector()){
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
            IntervalVector output_in(p->get_dim());
            IntervalVector output_out(p->get_dim());

            if(output){
                output_in = d->get_output_private();
                output_out = output_in;
                m_sep_output->separate(output_in, output_out);
                d->set_output_private(output_in & d->get_output_private());
            }
            else{
                output_in = d->get_input_private();
                output_out = output_in;
                m_sep_input->separate(output_in, output_out);
                d->set_input_private(output_in & d->get_input_private());
            }
            d->synchronize();
            if(!output_in.is_empty())
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

        if(position_in.is_empty()){
            contract_separator(maze, pave_node->get_children().first, true, list_pave_not_empty, output);
            contract_separator(maze, pave_node->get_children().second, true, list_pave_not_empty, output);
        }
        else if(position_out.is_empty()){
            pave_node->get_all_child_rooms(list_pave_not_empty, maze);
        }
        else{
            contract_separator(maze,pave_node->get_children().first, false, list_pave_not_empty, output);
            contract_separator(maze,pave_node->get_children().second, false, list_pave_not_empty, output);
        }

    }
}

}
