#ifndef ROOM_H
#define ROOM_H

#include "pave.h"
#include <ibex.h>

namespace invariant {

class Room
{
public:
    /**
     * @brief Room constructor
     * @param p
     * @param f_vect
     */
    Room(Pave *p, std::vector<ibex::Function *> f_vect);

    /**
     * @brief contract the pave (continuity + consistency)
     */
    void contract();

private:
    void contract_continuity();
    void contract_consistency();
    void contract_vector_field();

private:
    Pave*   m_pave; // pointer to the associated face
    std::vector<ibex::IntervalVector> m_vector_fields; // Vector field of the Room

    bool    m_empty;

};

}

#endif // ROOM_H
