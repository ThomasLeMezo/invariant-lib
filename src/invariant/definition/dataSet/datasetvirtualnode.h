#ifndef DATASETVIRTUALNODE_H
#define DATASETVIRTUALNODE_H

#include <vector>
#include <array>
#include <iostream>

using namespace std;

#define FUNCTION_EVAL_INVERT_BASE(type, cell) virtual bool eval_invert(std::vector<std::array<int, 2>> &target, const std::vector<std::array<int, 2>> &position, const std::array<std::array<type, 2>, cell>& data) const{return false;}
#define FUNCTION_EVAL_INVERT(type) \
    FUNCTION_EVAL_INVERT_BASE(type, 1)\
    FUNCTION_EVAL_INVERT_BASE(type, 2)\
    FUNCTION_EVAL_INVERT_BASE(type, 3)\
    FUNCTION_EVAL_INVERT_BASE(type, 4)\
    FUNCTION_EVAL_INVERT_BASE(type, 5)

#define FUNCTION_EVAL_BASE(type, cell) virtual void eval(const std::vector<std::array<int, 2>> &target, const std::vector<std::array<int, 2>> &position, std::array<std::array<type, 2>, cell>& data) const{}
#define FUNCTION_EVAL(type) \
    FUNCTION_EVAL_BASE(type, 1)\
    FUNCTION_EVAL_BASE(type, 2)\
    FUNCTION_EVAL_BASE(type, 3)\
    FUNCTION_EVAL_BASE(type, 4)\
    FUNCTION_EVAL_BASE(type, 5)

#define FUNCTION_NODE_VAL_BASE(type, cell) virtual void set_node_val(const std::array<std::array<type, 2>, cell> &data, bool valid_data=true){}
#define FUNCTION_NODE_VAL(type) \
    FUNCTION_NODE_VAL_BASE(type, 1)\
    FUNCTION_NODE_VAL_BASE(type, 2)\
    FUNCTION_NODE_VAL_BASE(type, 3)\
    FUNCTION_NODE_VAL_BASE(type, 4)\
    FUNCTION_NODE_VAL_BASE(type, 5)

class DataSetVirtualNode
{
public:

    DataSetVirtualNode(){}

    virtual ~DataSetVirtualNode() =0;

    /**
     * @brief Return true if the node is a leaf
     * @return
     */
    virtual bool is_leaf() const {return false;}

    /**
     * @brief Return true if the data is valid
     * @return
     */
    virtual bool is_valid_data() const  {return false;}

    /**
     * @brief Fill the tree
     * @return
     */
    virtual bool fill_tree()  {return false;}

    /**
     * @brief Eval the vector filed at this position
     * @param target
     * @param position
     * @param data
     */
    FUNCTION_EVAL(short int)
    FUNCTION_EVAL(int)
    FUNCTION_EVAL(double)
    FUNCTION_EVAL(char)

    /**
     * @brief Eval invert the vector filed at this position
     * @param target
     * @param position
     * @param data
     */
    FUNCTION_EVAL_INVERT(short int)
    FUNCTION_EVAL_INVERT(int)
    FUNCTION_EVAL_INVERT(double)
    FUNCTION_EVAL_INVERT(char)

    /**
     * @brief Set the node val
     * @param data
     * @param valid_data
     */
    FUNCTION_NODE_VAL(short int)
    FUNCTION_NODE_VAL(int)
    FUNCTION_NODE_VAL(double)
    FUNCTION_NODE_VAL(char)

    /**
     * @brief serialize
     * @param binFile
     */
    virtual void serialize(std::ofstream& binFile) const  {}

    /**
     * @brief get_number_node_private
     * @return
     */
    virtual size_t get_number_node() const {return 0;}

    /**
     * @brief get_number_leaf_private
     * @param nb
     */
    virtual size_t get_number_leaf() const {return 0;}

};

inline DataSetVirtualNode::~DataSetVirtualNode(){}

#endif // DATASETVIRTUALNODE_H
