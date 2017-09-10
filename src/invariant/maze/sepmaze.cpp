#include "sepmaze.h"

#include "room.h"

using namespace std;
using namespace ibex;

namespace invariant{

SepMaze::SepMaze(Maze *maze) : Sep(maze->get_graph()->dim()){
    m_maze = maze;
}

void SepMaze::separate(IntervalVector &in, IntervalVector &out){
    vector<Room *> list_room_not_empty, list_room_empty;
    m_maze->get_graph()->get_tree()->get_intersection_polygon_not_empty(list_room_not_empty, out, m_maze);
    m_maze->get_graph()->get_tree()->get_intersection_polygon_empty(list_room_empty, in, m_maze);

    IntervalVector in_tmp(in.size(), Interval::EMPTY_SET);
    IntervalVector out_tmp(out.size(), Interval::EMPTY_SET);

    for(Room* r:list_room_not_empty){
        out_tmp |= (r->get_hull() & out);
    }

    for(Room* r:list_room_empty){
        in_tmp |= (r->get_hull_complementary() & in);
        cout << in_tmp << endl;
    }
    in &= in_tmp;
    out &= out_tmp;
}

}
