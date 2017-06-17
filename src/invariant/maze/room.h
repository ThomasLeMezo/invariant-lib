#ifndef ROOM_H
#define ROOM_H

#include "pave.h"
#include "maze.h"
#include <ibex.h>

namespace invariant {

class Pave; // declared only for friendship
class Maze; // declared only for friendship
class Room
{
public:
    /**
     * @brief Room constructor
     * @param p
     * @param f_vect
     */
    Room(Pave *p, Maze *m, std::vector<ibex::Function *> f_vect);

    /**
     * @brief Room destructor
     */
    ~Room(){}

    /**
     * @brief contract the pave (continuity + consistency)
     */
    void contract();

    /**
     * @brief Getter to the associated Pave
     * @return
     */
    Pave* get_pave() const;

    /**
     * @brief Getter to the associated Maze
     * @return
     */
    Maze* get_maze() const;

private:
    void contract_continuity();
    void contract_consistency();
    void contract_vector_field();

private:
    Pave*   m_pave; // pointer to the associated face
    Maze*   m_maze; // pointer to the associated maze
    std::vector<ibex::IntervalVector> m_vector_fields; // Vector field of the Room

    bool    m_empty;

};
}

namespace invariant {
inline Pave* Room::get_pave() const{
    return m_pave;
}

inline Maze* Room::get_maze() const{
    return m_maze;
}
}

#endif // ROOM_H
