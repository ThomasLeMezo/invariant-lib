#include "booleantree.h"

namespace invariant {

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(const std::vector<Maze<_TpR,_TpF,_TpD>*> &maze_list){
    m_maze_list = maze_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(Maze<_TpR,_TpF,_TpD>* maze){
    m_maze_list.push_back(maze);
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(Maze<_TpR,_TpF,_TpD>* maze_A, Maze<_TpR,_TpF,_TpD>* maze_B){
    m_maze_list.push_back(maze_A);
    m_maze_list.push_back(maze_B);
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(Maze<_TpR,_TpF,_TpD>* maze_A, Maze<_TpR,_TpF,_TpD>* maze_B, Maze<_TpR,_TpF,_TpD>* maze_C){
    m_maze_list.push_back(maze_A);
    m_maze_list.push_back(maze_B);
    m_maze_list.push_back(maze_C);
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(const std::vector<BooleanTree<_TpR,_TpF,_TpD>*> &bisectionTree_list){
    m_children_list = bisectionTree_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree){
    m_children_list.push_back(bisectionTree);
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_B){
    m_children_list.push_back(bisectionTree_A);
    m_children_list.push_back(bisectionTree_B);
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_B, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_C){
    m_children_list.push_back(bisectionTree_A);
    m_children_list.push_back(bisectionTree_B);
    m_children_list.push_back(bisectionTree_C);
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(const std::vector<Maze<_TpR,_TpF,_TpD>*> &maze_list, const std::vector<BooleanTree<_TpR,_TpF,_TpD>*> &bisectionTree_list){
    m_children_list = bisectionTree_list;
    m_maze_list = maze_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(Maze<_TpR,_TpF,_TpD>* maze_A, BooleanTree<_TpR,_TpF,_TpD>* bisectionTree_A){
    m_maze_list.push_back(maze_A);
    m_children_list.push_back(bisectionTree_A);
}

template<typename _TpR, typename _TpF, typename _TpD>
BooleanTree<_TpR,_TpF,_TpD>::BooleanTree(BooleanTree<_TpR,_TpF,_TpD> &bisect_tree){
    m_children_list = bisect_tree.get_children_list();
    m_maze_list = bisect_tree.get_maze_list();
}

template<typename _TpR, typename _TpF, typename _TpD>
void BooleanTree<_TpR,_TpF,_TpD>::add_bisection_tree(BooleanTree<_TpR,_TpF,_TpD> * bisection_tree){
    m_children_list.push_back(bisection_tree);
}

template<typename _TpR, typename _TpF, typename _TpD>
void BooleanTree<_TpR,_TpF,_TpD>::add_maze(Maze<_TpR,_TpF,_TpD> * maze){
    m_maze_list.push_back(maze);
}

template<typename _TpR, typename _TpF, typename _TpD>
const std::vector<BooleanTree<_TpR,_TpF,_TpD>*>& BooleanTree<_TpR,_TpF,_TpD>::get_children_list() const{
    return m_children_list;
}

template<typename _TpR, typename _TpF, typename _TpD>
const std::vector<Maze<_TpR,_TpF,_TpD>*>& BooleanTree<_TpR,_TpF,_TpD>::get_maze_list() const{
    return m_maze_list;
}

}
