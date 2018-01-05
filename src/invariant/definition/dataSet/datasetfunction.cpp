#include "datasetfunction.h"
#include "datasetnode.h"
#include "../serialization/ibex_serialization.h"
#include "ibex_IntervalVector.h"

#include "omp.h"

using namespace std;

#define DATASETFUNCTION_CASE_INIT(Type, nb) \
    case nb:\
        m_root = new DataSetNode<Type, nb>(m_root_position, m_leaf_list);\
        break;
#define DATASETFUNCTION_SWITCH_INIT(Type) \
    switch(cell_dim){\
        DATASETFUNCTION_CASE_INIT(Type, 1)\
        DATASETFUNCTION_CASE_INIT(Type, 2)\
        DATASETFUNCTION_CASE_INIT(Type, 3)\
        DATASETFUNCTION_CASE_INIT(Type, 4)\
        DATASETFUNCTION_CASE_INIT(Type, 5)\
        break;\
    }\
    break;

#define DATASETFUNCTION_CASE_FILE(Type, nb) \
    case nb:\
        m_root = new DataSetNode<Type, nb>(binFile);\
        break;
#define DATASETFUNCTION_SWITCH_FILE(Type) \
    switch(m_cell_dim){\
        DATASETFUNCTION_CASE_FILE(Type, 1)\
        DATASETFUNCTION_CASE_FILE(Type, 2)\
        DATASETFUNCTION_CASE_FILE(Type, 3)\
        DATASETFUNCTION_CASE_FILE(Type, 4)\
        DATASETFUNCTION_CASE_FILE(Type, 5)\
        break;\
    }\
    break;

namespace invariant {

DataSetFunction::DataSetFunction(const ibex::IntervalVector& root_real_position, const std::vector<std::array<int, 2>> &root_position, const size_t cell_dim, const DataSetFunction_TYPE type){
    m_root_position = root_position;
    m_root_real_position = new ibex::IntervalVector(root_real_position);
    m_type = type;
    m_cell_dim = cell_dim;
    // BUILD THE TREE
    switch (type) {
    case TYPE_SHORT_INT:
        DATASETFUNCTION_SWITCH_INIT(short int);
        break;
    case TYPE_INT:
        DATASETFUNCTION_SWITCH_INIT(int);
        break;
    case TYPE_DOUBLE:
        DATASETFUNCTION_SWITCH_INIT(double);
        break;
    case TYPE_CHAR:
        DATASETFUNCTION_SWITCH_INIT(char);
        break;
    default:
        exit(EXIT_FAILURE);
        break;
    }
}

DataSetFunction::~DataSetFunction(){
    delete(m_root_real_position);
}

void DataSetFunction::append_data(){
    m_root->fill_tree();
}

const std::vector<std::array<int, 2>>& DataSetFunction::get_root_position() const{
    return m_root_position;
}

DataSetFunction::DataSetFunction(string file_name)
{
    std::ifstream binFile(file_name.c_str(), std::ofstream::in);

    /// ******** DataSetFunction INFOS ******** ///
    // Type
    binFile.read((char*)&m_type, sizeof(DataSetFunction_TYPE));

    // Cell Size
    binFile.read((char*)&m_cell_dim, sizeof(size_t));

    // Real Position
    m_root_real_position = new ibex::IntervalVector(deserializeIntervalVector(binFile));

    // Position
    size_t dim;
    binFile.read((char*)&dim, sizeof(size_t));
    for(size_t i = 0; i<dim; i++){
        std::array<int, 2> coord;
        binFile.read((char*)&coord, sizeof(std::array<int, 2>));
        m_root_position.push_back(coord);
    }

    /// ******** DataSetNode INFOS ******** ///
    switch(m_type){
    case TYPE_SHORT_INT:
        DATASETFUNCTION_SWITCH_FILE(short int);
        break;
    case TYPE_INT:
        DATASETFUNCTION_SWITCH_FILE(int);
        break;
    case TYPE_DOUBLE:
        DATASETFUNCTION_SWITCH_FILE(double);
        break;
    case TYPE_CHAR:
        DATASETFUNCTION_SWITCH_FILE(char);
        break;
    default:
        break;
    }

    binFile.close();
}

void DataSetFunction::serialize(string file_name) const{
    std::ofstream binFile(file_name.c_str(), std::ofstream::out);
    /// ******** DataSetFunction INFOS ******** ///
    // Type
    binFile.write((const char*)&m_type, sizeof(DataSetFunction_TYPE));

    // Cell Size
    binFile.write((const char*)&m_cell_dim, sizeof(size_t));

    // Real Position
    serializeIntervalVector(binFile, *m_root_real_position);

    // Position
    size_t dim = m_root_position.size();
    binFile.write((const char*) dim, sizeof(size_t));
    for(const std::array<int, 2> &a:m_root_position){
        binFile.write((const char*)&a, sizeof(std::array<int, 2>));
    }

    /// ******** DataSetNode INFOS ******** ///
    m_root->serialize(binFile);
    binFile.close();
}

ibex::Interval DataSetFunction::eval(const ibex::IntervalVector& box) const{
    return ibex::Interval(ibex::Interval::EMPTY_SET);
}
ibex::IntervalVector DataSetFunction::eval_vector(const ibex::IntervalVector& box) const{
    return ibex::IntervalVector(box.size(), ibex::Interval::EMPTY_SET);
}
ibex::IntervalMatrix DataSetFunction::eval_matrix(const ibex::IntervalVector& box) const{
    return ibex::IntervalMatrix(box.size(), box.size(), ibex::Interval::EMPTY_SET);
}
}
