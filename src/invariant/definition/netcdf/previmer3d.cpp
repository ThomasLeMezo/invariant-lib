#include "previmer3d.h"
#include "dataSet/datasetnode.h"
#include "dataSet/datasetfunction.h"

#include <netcdf>
#include <omp.h>

#include <sys/types.h>
#include <dirent.h>

#include <utility>
#include <algorithm>

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <fstream>

#include "../serialization/ibex_serialization.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
namespace pt = boost::property_tree;

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;
using namespace ibex;

namespace invariant {

int parseLine(char* line)
{
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getRAM()
{ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != nullptr){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

void PreviMer3D::load_data(const std::string &file_xml, std::vector<std::vector<std::vector<short int>>> &raw_u_t,
                           std::vector<std::vector<std::vector<short int>>> &raw_v_t,
                           const std::array<std::array<size_t, 2>, 2> &grid_limits){
    pt::ptree tree;
    cout << " READ file_xml = " << file_xml << endl;
    pt::read_xml(file_xml, tree);

    string directory = tree.get<string>("PREVIMER.<xmlattr>.directory");
    cout << " DATA Directory = " << directory<< endl;
    int file_id = 0;
    bool first_read = true;

    size_t i_max_save =0, j_max_save=0;

    BOOST_FOREACH(pt::ptree::value_type &v, tree.get_child("PREVIMER")){
        if(v.first == "file"){
            string file_name = v.second.get<string>("");
            if(!file_name.empty()){
                cout << " " << file_id << " " << file_name << endl;
                file_id++;

                NcFile dataFile(directory+file_name, NcFile::read);

                NcVar u_var=dataFile.getVar("U");
                NcVar v_var=dataFile.getVar("V");

                // ******* Size U,V ******
                size_t j_max_file = dataFile.getDim("nj_u").getSize();
                size_t i_max_file = dataFile.getDim("ni_u").getSize();

                m_offset_i = std::max((long unsigned int)0, grid_limits[0][0]);
                m_offset_j = std::max((long unsigned int)0, grid_limits[1][0]);

                // ******* DATA U,V ******
                short int *raw_u = new short int[j_max_file*i_max_file];
                short int *raw_v = new short int[j_max_file*i_max_file];
                u_var.getVar(raw_u);
                v_var.getVar(raw_v);

                vector<vector<short int>> tab_u, tab_v;
                size_t i_max = min(i_max_file, grid_limits[0][1])-m_offset_i;
                size_t j_max = min(j_max_file, grid_limits[1][1])-m_offset_j;
                for(size_t i=m_offset_i; i<i_max+m_offset_i; i++){
                    vector<short int> line_u, line_v;
                    for(size_t j=m_offset_j; j<j_max+m_offset_j; j++){
                        line_u.push_back(raw_u[j*i_max_file+i]); // y, x tab
                        line_v.push_back(raw_v[j*i_max_file+i]); // y, x tab
                    }
                    tab_u.push_back(line_u);
                    tab_v.push_back(line_v);
                }

                delete[] raw_u;
                delete[] raw_v;

                raw_u_t.push_back(tab_u);
                raw_v_t.push_back(tab_v);

                // Global parameters
                if(first_read){
                    u_var.getAtt("scale_factor").getValues(&m_scale_factor);
                    u_var.getAtt("_FillValue").getValues(&m_fill_value);
                    u_var.getAtt("valid_min").getValues(&m_min_valid);
                    u_var.getAtt("valid_max").getValues(&m_max_valid);
                    i_max_save = i_max;
                    j_max_save = j_max;
                    first_read = false;
                }
            }
        }
    }
    std::array<int, 2> root_t = {0, file_id-1};
    std::array<int, 2> root_i = {0, (int)i_max_save-1};
    std::array<int, 2> root_j = {0, (int)j_max_save-1};
    m_node_root_position.push_back(root_t);
    m_node_root_position.push_back(root_i);
    m_node_root_position.push_back(root_j);

    m_search_space[0] = Interval(0, file_id*m_grid_conversion[0]);
    m_search_space[1] = Interval(m_offset_i*m_grid_conversion[1], (m_offset_i+i_max_save)*m_grid_conversion[1]);
    m_search_space[2] = Interval(m_offset_j*m_grid_conversion[2], (m_offset_j+j_max_save)*m_grid_conversion[2]);
}

PreviMer3D::PreviMer3D(const std::string& file_xml, const std::array<std::array<size_t, 2>, 2> &grid_limits):
    m_search_space(3)
{
//    m_sens_previmer = sens;
    int ram_init = getRAM()/1000;
    cout << "Mem 0 = " << ram_init << " Mo" << endl;

    /// **************** LOAD DATA **************** //
    cout << "**** LOAD DATA ****" << endl;
    std::vector<std::vector<std::vector<short int>>> raw_u_t;
    std::vector<std::vector<std::vector<short int>>> raw_v_t;
    load_data(file_xml, raw_u_t, raw_v_t, grid_limits);

    cout << "-> Data properties" << endl;
    cout << "-> Scale factor = " << m_scale_factor << endl;
    cout << "-> Fill Value = " << m_fill_value << endl;

    double time_start_init = omp_get_wtime();
    cout << "-> size raw_u_t = " << raw_u_t.size() << " " << raw_u_t[0].size() << " " << raw_u_t[0][0].size() << endl;
    cout << "-> MEMORY 1 = " << getRAM()/1000 - ram_init << " Mo" << endl;  // 29 Mo

    /// **************** BUILD THE TREE **************** //

    cout << "**** BUILD THE TREE ****" << endl;
    m_node_current = new DataSetNode<short int, 2>(m_node_root_position, m_leaf_list);

    cout << "-> Nb leafs = " << m_leaf_list.size() << endl;
    cout << "-> TIME build the tree " << omp_get_wtime() - time_start_init << endl;
    time_start_init = omp_get_wtime();
    cout << "-> MEMORY 2 = " << getRAM()/1000 - ram_init << " Mo" << endl;

    /// **************** FILL THE LEAFS **************** //
    cout << "**** FILL THE LEAFS ****" << endl;
    fill_leafs(raw_u_t, raw_v_t);

    cout << "-> TIME fill the leafs " << omp_get_wtime() - time_start_init << endl;
    time_start_init = omp_get_wtime();
    cout << "-> MEMORY 3 = " << getRAM()/1000 - ram_init<< " Mo" << endl;

    /// **************** FILL THE TREE **************** //
    cout << "**** FILL THE TREE ****" << endl;
    time_start_init = omp_get_wtime();
    m_node_current->fill_tree();

    cout << "-> TIME fill the tree = " << omp_get_wtime() - time_start_init << endl;
    cout << "-> MEMORY 4 = " << getRAM()/1000 - ram_init << " Mo" << endl;

    serialize("PreviMer3D.data");
}

PreviMer3D::~PreviMer3D()
{
    delete(m_node_current);
}

void PreviMer3D::serialize(const string &file_name){
    std::ofstream binFile(file_name.c_str(), std::ofstream::out);

    /// ******** Previmer3D variable ******** ///

    serializeIntervalVector(binFile, m_search_space);
    binFile.write((const char*)&m_scale_factor, sizeof(float));
    binFile.write((const char*)&m_fill_value, sizeof(short));
    binFile.write((const char*)&m_min_valid, sizeof(short));
    binFile.write((const char*)&m_max_valid, sizeof(short));

    serializeVector<double>(m_grid_conversion, binFile);

    binFile.write((const char*)&m_offset_i, sizeof(size_t));
    binFile.write((const char*)&m_offset_j, sizeof(size_t));

    serializeVector<std::array<int, 2>>(m_node_root_position, binFile);

    m_node_current->serialize(binFile);
    binFile.close();
}

PreviMer3D::PreviMer3D(const std::string& file_name):
    m_search_space(3)
{
    int ram_init = getRAM()/1000;
    std::ifstream binFile(file_name.c_str(), std::ifstream::in);

    /// ******** Previmer3D variable ******** ///

    m_search_space = deserializeIntervalVector(binFile);
    binFile.read((char*)&m_scale_factor, sizeof(float));
    binFile.read((char*)&m_fill_value, sizeof(short));
    binFile.read((char*)&m_min_valid, sizeof(short));
    binFile.read((char*)&m_max_valid, sizeof(short));

    m_grid_conversion = deserializeVector<double>(binFile);

    binFile.read((char*)&m_offset_i, sizeof(size_t));
    binFile.read((char*)&m_offset_j, sizeof(size_t));

    m_node_root_position = deserializeVector<std::array<int, 2>>(binFile);

    m_node_current = new DataSetNode<short int, 2>(binFile);
    binFile.close();
    cout << "-> MEMORY = " << getRAM()/1000 - ram_init << " Mo" << endl;
}

ibex::IntervalVector PreviMer3D::eval_vector(const ibex::IntervalVector& position) const{
    // Apply function of conversion raster <--> Reality
    std::vector<std::array<int, 2>> target;

    array<int, 2> t = {(int)floor(position[0].lb()/m_grid_conversion[0]), (int)floor(position[0].ub()/m_grid_conversion[0])};
    array<int, 2> i = {(int)floor(position[1].lb()/m_grid_conversion[1]-m_offset_i), (int)floor(position[1].ub()/m_grid_conversion[1]-m_offset_i)};
    array<int, 2> j = {(int)floor(position[2].lb()/m_grid_conversion[2]-m_offset_j), (int)floor(position[2].ub()/m_grid_conversion[2]-m_offset_j)};
    target.push_back(t);
    target.push_back(i);
    target.push_back(j);

    std::array<std::array<signed short int, 2>, 2> data;
    data[0][0] = m_max_valid;
    data[0][1] = m_min_valid;
    data[1][0] = m_max_valid;
    data[1][1] = m_min_valid;
    m_node_current->eval(target, m_node_root_position, data);

    IntervalVector vec(3);
    if((data[0][0] >= m_max_valid && data[0][1] <= m_min_valid)
            || (data[1][0] >= m_max_valid && data[1][1] <= m_min_valid)){
        vec = IntervalVector(3, Interval::EMPTY_SET);
    }
    else{
        vec[0] = Interval(1); // in s
        vec[1] = Interval(data[0][0] * m_scale_factor, data[0][1] * m_scale_factor); // in m
        vec[2] = Interval(data[1][0] * m_scale_factor, data[1][1] * m_scale_factor); // in m
    }
    return vec;
}

void PreviMer3D::fill_leafs(const std::vector<std::vector<std::vector<short int> > > &raw_u_t, const std::vector<std::vector<std::vector<short int> > > &raw_v_t)
{
    int nb_node=m_leaf_list.size();

    #pragma omp parallel for //num_threads(1)
    for(int id=0; id<nb_node; id++){
        DataSetNode<short int, 2> *rt = (DataSetNode<short int, 2>*)(m_leaf_list[id].first);
        // [x], [y], [t] => array value should be identical
        std::vector<std::array<int, 2>> position = m_leaf_list[id].second;

        // => Set without any error
        std::array<std::array<signed short int, 2>, 2> data;
        data[0][0] = raw_u_t[position[0][0]][position[1][0]][position[2][0]]; // t, y, x tab ?
        data[0][1] = data[0][0];
        data[1][0] = raw_v_t[position[0][0]][position[1][0]][position[2][0]]; // t, y, x tab ?
        data[1][1] = data[1][0];
        bool valid_data = true;

        if(data[0][0] < m_min_valid || data[1][0] < m_min_valid
           || data[0][1] > m_max_valid || data[1][1] > m_max_valid)
            valid_data = false;

        rt->set_node_val(data, valid_data);
    }
}

}
