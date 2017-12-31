#include "sepmaze.h"

namespace invariant{

template <typename _Tp>
SepMaze<_Tp>::SepMaze(Maze<_Tp> *maze) : Sep(maze->get_subpaving()->dim()){
    m_maze = maze;
}

template <typename _Tp>
void SepMaze<_Tp>::separate(ibex::IntervalVector &in, ibex::IntervalVector &out){
    vector<Room<_Tp> *> list_room_not_empty, list_room_empty;
    m_maze->get_subpaving()->get_tree()->get_intersection_polygon_not_empty(list_room_not_empty, out, m_maze);
    m_maze->get_subpaving()->get_tree()->get_intersection_polygon_empty(list_room_empty, in, m_maze);

    ibex::IntervalVector in_tmp(in.size(), ibex::Interval::EMPTY_SET);
    ibex::IntervalVector out_tmp(out.size(), ibex::Interval::EMPTY_SET);

    for(Room<_Tp>* r:list_room_not_empty){
        out_tmp |= (r->get_hull() & out);
    }

    for(Room<_Tp>* r:list_room_empty){
        in_tmp |= (r->get_hull_complementary() & in);
        std::cout << in_tmp << std::endl;
    }
    in &= in_tmp;
    out &= out_tmp;
}

}
