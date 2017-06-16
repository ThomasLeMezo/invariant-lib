#include "room.h"

using namespace ibex;
namespace invariant {

Room::Room(Pave *p, std::vector<ibex::Function*> f_vect)
{
    m_pave = p;
    const IntervalVector position(p->get_position());
    for(Function*f:f_vect){
        IntervalVector vector_field = f->eval_vector(position);
        m_vector_fields.push_back(vector_field);
    }
}

}
