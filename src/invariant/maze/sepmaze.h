#ifndef SEPMAZE_H
#define SEPMAZE_H

#include "ibex/ibex_Sep.h"
#include "ibex/ibex_IntervalVector.h"
#include "maze.h"

namespace invariant {

class SepMaze : public ibex::Sep
{
public:

    /**
     * @brief Constructor of the Maze separator
     * @param maze
     */
    SepMaze(Maze *maze);

    /**
     * @brief Separator to the maze polygon
     * @param box
     */
    virtual void separate(ibex::IntervalVector& x_in, ibex::IntervalVector& x_out);

private:
    Maze *m_maze;
};

}

#endif // SEPMAZE_H
