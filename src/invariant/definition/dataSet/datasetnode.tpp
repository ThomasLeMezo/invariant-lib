#include "datasetnode.h"
#include <math.h>
#include <limits>
#include <vector>
#include <array>
#include <iostream>

#include <algorithm>
#include <iterator>

using namespace std;

namespace invariant {

template<typename _Tp, size_t _n>
DataSetNode<_Tp, _n>::DataSetNode(const std::vector<std::array<int, 2> > &position, std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > > &leaf_list)
{
    for(size_t dim=0; dim<_n; dim++){
        m_data[dim][0] = std::numeric_limits<_Tp>::max();
        m_data[dim][1] = std::numeric_limits<_Tp>::min();
    }

    bool limit_reach = true;
    for(size_t dim=0; dim<position.size(); dim++){
        if(position[dim][0] < position[dim][1]){
            limit_reach = false;
            break;
        }
    }

    if(limit_reach){
        // One pixel size
        m_bisection_axis = -1;
        m_children_first = nullptr;
        m_children_second = nullptr;
        std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > pair(this, position);
        leaf_list.push_back(pair);
    }
    else{
        // Bisect
        std::vector<std::array<int, 2>> p1, p2;
        m_bisection_axis = (signed char) bisector(position, p1, p2);

        DataSetNode<_Tp, _n> *rt1 = new DataSetNode<_Tp, _n>(p1, leaf_list);
        DataSetNode<_Tp, _n> *rt2 = new DataSetNode<_Tp, _n>(p2, leaf_list);
        m_children_first = rt1;
        m_children_second = rt2;
    }
}

template<typename _Tp, size_t _n>
DataSetNode<_Tp, _n>::~DataSetNode(){
    if(!is_leaf()){
        if(m_children_first!=nullptr)
            delete(m_children_first);
        if(m_children_second!=nullptr)
            delete(m_children_second);
    }
}

template<typename _Tp, size_t _n>
size_t DataSetNode<_Tp, _n>::bisector(const std::vector<std::array<int, 2>> &position,std::vector<std::array<int, 2>> &p1,std::vector<std::array<int, 2>> &p2) const
{
    p1.clear(); p2.clear();

    // Find largest dim
    size_t l_dim = 0;
    int l_size = position[0][1]-position[0][0];
    for(size_t dim=1; dim<position.size(); dim++){
        int comp_size = position[dim][1]-position[dim][0];
        if(comp_size>l_size){
            l_size = comp_size;
            l_dim = dim;
        }
    }

    for(size_t dim=0; dim<position.size(); dim++){
        if(dim==l_dim){
            array<int, 2> a1, a2;
            a1[0] = position[dim][0];
            a1[1] = position[dim][0] + (position[dim][1]-position[dim][0])/2;
            a2[0] = a1[1]+1;
            a2[1] = position[dim][1];
            p1.push_back(a1);
            p2.push_back(a2);
        }
        else{
            p1.push_back(position[dim]);
            p2.push_back(position[dim]);
        }
    }
    return l_dim;
}

template<typename _Tp, size_t _n>
bool DataSetNode<_Tp, _n>::fill_tree(){
    if(is_leaf())
        return m_valid_data;
    else{
        bool valid_data = false;
        valid_data |= m_children_first->fill_tree();
        valid_data |= m_children_second->fill_tree();

        if(m_children_first->is_valid_data()){
            union_this(m_children_first->get_data());
        }
        if(m_children_second->is_valid_data()){
            union_this(m_children_second->get_data());
        }

        if(m_children_first->is_leaf() && m_children_second->is_leaf()){
            if(m_children_first->get_data()==m_children_second->get_data()){
                m_bisection_axis=-1;

                delete(m_children_first);
                delete(m_children_second);

                m_children_first = nullptr;
                m_children_second = nullptr;
            }
        }

        m_valid_data = valid_data;
        return valid_data;
    }
}

template<typename _Tp, size_t _n>
void DataSetNode<_Tp, _n>::eval(const std::vector<std::array<int, 2>> &target, const std::vector<std::array<int, 2>> &position, std::array<std::array<_Tp, 2>, _n>& data) const{
    if(is_inter_empty(target, position)){
        return;
    }
    else if(is_leaf() || is_subset(position, target)){
        if(m_valid_data)
            union_data(data);
    }
    else{
        // Bisect
        if(m_valid_data){
            // Test Data inclusion
            bool inclusion = true;
            for(size_t dim=0; dim<_n; dim++){
                if(data[dim][0] > m_data[dim][0] && data[dim][1] < m_data[dim][1]) // complementary
                    inclusion = false;
            }
            if(inclusion)
                return;

            std::vector<std::array<int, 2>> p1, p2;
            bisector(position, p1, p2);
            if(m_children_first != nullptr)
                m_children_first->eval(target, p1, data);
            if(m_children_second != nullptr)
                m_children_second->eval(target, p2, data);
        }
    }
}

template<typename _Tp, size_t _n>
void DataSetNode<_Tp, _n>::union_this(const std::array<std::array<_Tp, 2>, _n>& data){
    for(size_t dim=0; dim<_n; dim++){
        m_data[dim][0] = min(m_data[dim][0], data[dim][0]);
        m_data[dim][1] = max(m_data[dim][1], data[dim][1]);
    }
}

template<typename _Tp, size_t _n>
void DataSetNode<_Tp, _n>::union_data(std::array<std::array<_Tp, 2>, _n>& data) const{
    for(size_t dim=0; dim<_n; dim++){
        data[dim][0] = min(m_data[dim][0], data[dim][0]);
        data[dim][1] = max(m_data[dim][1], data[dim][1]);
    }
}

template<typename _Tp, size_t _n>
bool DataSetNode<_Tp, _n>::is_inter_empty(const std::vector<std::array<int, 2>> &p1, const std::vector<std::array<int, 2>> &p2) const{
    for(size_t dim = 0; dim<p1.size(); dim++){
        if(p1[dim][1]<p2[dim][0] || p2[dim][1]<p1[dim][0])
            return true;
    }
    return false;
}

template<typename _Tp, size_t _n>
bool DataSetNode<_Tp, _n>::is_subset(const std::vector<std::array<int, 2>> &p1, const std::vector<std::array<int, 2>> &p2) const{
    // p1 subset of p2
    for(size_t dim = 0; dim<p1.size(); dim++){
        if(p1[dim][1]>p2[dim][1] || p1[dim][0]<p2[dim][0])
            return false;
    }
    return true;
}

template<typename _Tp, size_t _n>
void DataSetNode<_Tp, _n>::serialize(std::ofstream& binFile) const{
    binFile.write((const char*) &m_bisection_axis, sizeof(signed char));
    binFile.write((const char*) &m_valid_data, sizeof(bool));
    binFile.write((const char*) &m_data, sizeof(std::array<std::array<_Tp, 2>, _n>));

    bool val_true = true;
    bool val_false = false;

    if(m_children_first != nullptr){
        binFile.write((const char*) &val_true, sizeof(bool));
        m_children_first->serialize(binFile);
    }
    else{
        binFile.write((const char*) &val_false, sizeof(bool));
    }

    if(m_children_second != nullptr){
        binFile.write((const char*) &val_true, sizeof(bool));
        m_children_second->serialize(binFile);
    }
    else{
        binFile.write((const char*) &val_false, sizeof(bool));
    }
}

template<typename _Tp, size_t _n>
DataSetNode<_Tp, _n>::DataSetNode(std::ifstream& binFile){
    binFile.read((char*) &m_bisection_axis, sizeof(signed char));
    binFile.read((char*) &m_valid_data, sizeof(bool));
    binFile.read((char*) &m_data, sizeof(std::array<std::array<_Tp, 2>, _n>));

    bool is_children_first;
    binFile.read((char*) &is_children_first, sizeof(bool));
    if(is_children_first){
        m_children_first = new DataSetNode<_Tp, _n>(binFile);
    }

    bool is_children_second;
    binFile.read((char*) &is_children_second, sizeof(bool));
    if(is_children_second){
        m_children_second = new DataSetNode<_Tp, _n>(binFile);
    }
}

}

