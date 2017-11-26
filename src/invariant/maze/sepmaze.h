#ifndef SEPMAZE_H
#define SEPMAZE_H

#include "ibex/ibex_Sep.h"
#include "ibex/ibex_IntervalVector.h"

#include "maze.h"

namespace invariant {

template <typename _Tp> class Maze;

template <typename _Tp=ibex::IntervalVector>
class SepMaze : public ibex::Sep
{
public:

    /**
     * @brief Constructor of the Maze separator
     * @param maze
     */
    SepMaze(Maze<_Tp> *maze);

    /**
     * @brief Separator to the maze polygon
     * @param box
     */
    virtual void separate(ibex::IntervalVector& x_in, ibex::IntervalVector& x_out);

private:
    Maze<_Tp> *m_maze;
};

}

#include "sepmaze.tpp"

#endif // SEPMAZE_H
