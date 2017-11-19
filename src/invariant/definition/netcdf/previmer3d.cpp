#include "previmer3d.h"
#include "rasterTree/rastertree.h"

#include <netcdf>
#include <omp.h>

#include <sys/types.h>
#include <dirent.h>

#include <utility>
#include <algorithm>

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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

    while (fgets(line, 128, file) != NULL){
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

    size_t i_max_save, j_max_save;

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
                        line_u.push_back(raw_u[i_max_file*j+i]);
                        line_v.push_back(raw_v[j_max_file*j+i]);
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
    std::array<int, 2> root_i = {0, i_max_save-1};
    std::array<int, 2> root_j = {0, j_max_save-1};
    m_node_root_position.push_back(root_t);
    m_node_root_position.push_back(root_i);
    m_node_root_position.push_back(root_j);

    m_search_space[0] = Interval(0, file_id*m_grid_conversion[0]);
    m_search_space[1] = Interval(m_offset_i*m_grid_conversion[1], (m_offset_i+i_max_save)*m_grid_conversion[1]);
    m_search_space[2] = Interval(m_offset_j*m_grid_conversion[2], (m_offset_j+j_max_save)*m_grid_conversion[2]);
}

PreviMer3D::PreviMer3D(const std::string& file_xml, const std::array<std::array<size_t, 2>, 2> &grid_limits):
    Dynamics(FWD), m_search_space(3)
{
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
    m_node_current = new RasterTree<short int, 2>(m_node_root_position, m_leaf_list, m_leaf_position);

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
    short int val_min[2],val_max[2];
    val_min[0] = m_max_valid; // Invert min/max bounds to fill the tree
    val_min[1] = m_max_valid;
    val_max[0] = m_min_valid;
    val_max[1] = m_min_valid;
    m_node_current->fill_tree(m_node_root_position, val_min, val_max);

    cout << "-> TIME fill the tree = " << omp_get_wtime() - time_start_init << endl;
    cout << "-> MEMORY 4 = " << getRAM()/1000 - ram_init << " Mo" << endl;
}

PreviMer3D::~PreviMer3D()
{
    delete(m_node_current);
}

const vector<ibex::IntervalVector> PreviMer3D::eval(const ibex::IntervalVector& position){
    vector<ibex::IntervalVector> vector_fields;

    // Apply function of conversion raster <--> Reality
    std::vector<std::array<int, 2>> target;

    array<int, 2> t = {floor(position[0].lb()/m_grid_conversion[0]), ceil(position[0].ub()/m_grid_conversion[0])};
    array<int, 2> i = {floor(position[1].lb()/m_grid_conversion[1]-m_offset_i), ceil(position[1].ub()/m_grid_conversion[1]-m_offset_i)};
    array<int, 2> j = {floor(position[2].lb()/m_grid_conversion[2]-m_offset_j), ceil(position[2].ub()/m_grid_conversion[2]-m_offset_j)};
    target.push_back(t);
    target.push_back(i);
    target.push_back(j);

    signed short int val_min[2],val_max[2];
    val_min[0] = m_max_valid;
    val_min[1] = m_max_valid;
    val_max[0] = m_min_valid;
    val_max[1] = m_min_valid;
    m_node_current->eval(target, m_node_root_position, val_min, val_max);

    IntervalVector vec(3);
    if((val_min[0] >= m_max_valid && val_max[0] <= m_min_valid)
            || (val_min[1] >= m_max_valid && val_max[1] <= m_min_valid)){
        vec = IntervalVector(3, Interval::EMPTY_SET);
    }
    else{
        vec[0] = Interval(0.9999, 1.0);
        vec[1] = Interval(val_min[0] * m_scale_factor, val_max[0] * m_scale_factor) + Interval(-0.1, 0.1);
        vec[2] = Interval(val_min[1] * m_scale_factor, val_max[1] * m_scale_factor) + Interval(-0.1, 0.1);
    }
    vector_fields.push_back(vec);
    return vector_fields;
}

void PreviMer3D::fill_leafs(const std::vector<std::vector<std::vector<short int> > > &raw_u_t, const std::vector<std::vector<std::vector<short int> > > &raw_v_t)
{
    int nb_node=m_leaf_list.size();

    #pragma omp parallel for num_threads(1)
    for(int id=0; id<nb_node; id++){
        RasterTree<short int, 2> *rt = m_leaf_list[id];
        // [x], [y], [t] => array value should be identical
        std::vector<std::array<int, 2>> position = m_leaf_position[id];

        if(position[0][0] == 0 && position[1][0]==209 && position[2][0]==399)
            cout << "debug" << endl;

        // => Set without any error
        short int val_min[2], val_max[2];
        val_min[0] = raw_u_t[position[0][0]][position[1][0]][position[2][0]];
        val_min[1] = raw_v_t[position[0][0]][position[1][0]][position[2][0]];
        val_max[0] = val_min[0];
        val_max[1] = val_min[1];
        bool valid_data = true;

        if(val_min[0] < m_min_valid || val_min[1] < m_min_valid
           || val_max[0] > m_max_valid || val_max[1] > m_max_valid)
            valid_data = false;

        rt->set_node_val(val_min, val_max, valid_data);
    }
}

}
