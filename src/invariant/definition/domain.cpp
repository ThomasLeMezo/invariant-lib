#include "domain.h"

using namespace ibex;
using namespace std;
namespace invariant {

Domain::Domain(Graph *graph){
    m_graph = graph;
}

void Domain::contract_separator(Maze *maze, Pave_node *pave_node, bool all_out, std::vector<Pave*> &l){
    if(all_out){
        if(pave_node->is_leaf()){
            Pave* p=pave_node->get_pave();
            for(Face * f:p->get_faces_vector()){
                Door *d = f->get_doors()[maze];
                d->set_empty_output();
                d->synchronize();
            }
        }
        else{
            contract_separator(maze, pave_node->get_children().first, true, l);
            contract_separator(maze, pave_node->get_children().second, true, l);
        }
    }
    else if(pave_node->is_leaf()){
        Pave* p=pave_node->get_pave();
        for(Face * f:p->get_faces_vector()){
            Door *d = f->get_doors()[maze];
            IntervalVector output_in(d->get_output_private());
            IntervalVector output_out(output_in);

            for(Sep* sep:m_sep_input){
                sep->separate(output_in, output_out);
            }
            d->set_output_private(output_in);
            d->synchronize();
        }
    }
    else{
        IntervalVector position_in(pave_node->get_position());
        IntervalVector position_out(position_in);
        for(Sep* sep:m_sep_input){
            sep->separate(position_in, position_out);
        }
        if(position_in.is_empty()){
            contract_separator(maze, pave_node->get_children().first, true, l);
            contract_separator(maze, pave_node->get_children().second, true, l);
        }
        else if(position_out.is_empty()){
            return;
        }
        else{
            contract_separator(maze,pave_node->get_children().first, false, l);
            contract_separator(maze,pave_node->get_children().second, false, l);
        }

    }
}

}
