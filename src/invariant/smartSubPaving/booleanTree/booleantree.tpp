#include "booleantree.h"

namespace invariant {

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(const std::vector<Maze<_Tp>*> &maze_list){
    m_maze_list = maze_list;
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(Maze<_Tp>* maze){
    m_maze_list.push_back(maze);
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B){
    m_maze_list.push_back(maze_A);
    m_maze_list.push_back(maze_B);
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(Maze<_Tp>* maze_A, Maze<_Tp>* maze_B, Maze<_Tp>* maze_C){
    m_maze_list.push_back(maze_A);
    m_maze_list.push_back(maze_B);
    m_maze_list.push_back(maze_C);
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(const std::vector<BooleanTree<_Tp>*> &bisectionTree_list){
    m_children_list = bisectionTree_list;
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(BooleanTree<_Tp>* bisectionTree){
    m_children_list.push_back(bisectionTree);
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B){
    m_children_list.push_back(bisectionTree_A);
    m_children_list.push_back(bisectionTree_B);
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(BooleanTree<_Tp>* bisectionTree_A, BooleanTree<_Tp>* bisectionTree_B, BooleanTree<_Tp>* bisectionTree_C){
    m_children_list.push_back(bisectionTree_A);
    m_children_list.push_back(bisectionTree_B);
    m_children_list.push_back(bisectionTree_C);
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(const std::vector<Maze<_Tp>*> &maze_list, const std::vector<BooleanTree<_Tp>*> &bisectionTree_list){
    m_children_list = bisectionTree_list;
    m_maze_list = maze_list;
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(Maze<_Tp>* maze_A, BooleanTree<_Tp>* bisectionTree_A){
    m_maze_list.push_back(maze_A);
    m_children_list.push_back(bisectionTree_A);
}

template<typename _Tp>
BooleanTree<_Tp>::BooleanTree(BooleanTree<_Tp> &bisect_tree){
    m_children_list = bisect_tree.get_children_list();
    m_maze_list = bisect_tree.get_maze_list();
}

template<typename _Tp>
void BooleanTree<_Tp>::add_bisection_tree(BooleanTree<_Tp> * bisection_tree){
    m_children_list.push_back(bisection_tree);
}

template<typename _Tp>
void BooleanTree<_Tp>::add_maze(Maze<_Tp> * maze){
    m_maze_list.push_back(maze);
}

template<typename _Tp>
const std::vector<BooleanTree<_Tp>*>& BooleanTree<_Tp>::get_children_list() const{
    return m_children_list;
}

template<typename _Tp>
const std::vector<Maze<_Tp>*>& BooleanTree<_Tp>::get_maze_list() const{
    return m_maze_list;
}

}
