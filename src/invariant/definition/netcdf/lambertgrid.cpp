#include "lambertgrid.h"
#include <netcdf>
#include <omp.h>
#include <proj_api.h>
#include <iostream>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
namespace pt = boost::property_tree;

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

namespace invariant {

void load_raw_data(const NcFile &dataFile, const string &var_name, double *data, const size_t &data_size){
    NcVar var=dataFile.getVar(var_name);

    double scale_factor, add_offset;
    var.getAtt("scale_factor").getValues(&scale_factor);
    var.getAtt("add_offset").getValues(&add_offset);

    short int *raw = new short int[data_size];
    var.getVar(raw);

    for(size_t i=0; i<data_size; i++)
        data[i] = (scale_factor*raw[i]+add_offset)*DEG_TO_RAD;

    delete[] raw;
}

void fill_vector(std::vector<std::vector<double>> &X, std::vector<std::vector<double>>&Y, double *dataX, double *dataY, const size_t &ni, const size_t &nj){
    for(size_t i=0; i<ni; i++){
        X.push_back(vector<double>());
        Y.push_back(vector<double>());
        for(size_t j=0; j<nj; j++){
            X[i].push_back(dataX[j*ni+i]);
            Y[i].push_back(dataY[j*ni+i]);
        }
    }
}

void LambertGrid::fill_leaf(std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2>>>> &leaf_list,
               std::vector<std::vector<double>> &X,
               std::vector<std::vector<double>> &Y){
    for(std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > &leaf:leaf_list){
        vector<array<int, 2>> position = leaf.second;
        array<array<double, 2>, 2> data;
        double x_center = X[position[0][0]][position[1][0]];
        double x_ub, x_lb;
        if(position[0][0]+1<(int)X.size())
            x_ub = x_center + ceil(abs(X[position[0][0]+1][position[1][0]]-x_center)/2.0);
        else
            x_ub = x_center;

        if(position[0][0]-1>=0)
            x_lb = x_center - ceil(abs(x_center-X[position[0][0]-1][position[1][0]])/2.0);
        else
            x_lb = x_center;

        double y_center = Y[position[0][0]][position[1][0]];
        double y_ub, y_lb;
        if(position[1][0]+1<(int)Y[position[0][0]].size())
            y_ub = y_center + ceil(abs(Y[position[0][0]][position[1][0]+1]-y_center)/2.0);
        else
            y_ub = y_center;

        if(position[1][0]-1>=0)
            y_lb = y_center - ceil(abs(y_center-Y[position[0][0]][position[1][0]-1])/2.0);
        else
            y_lb = y_center;

        data[0][0] = x_lb; data[0][1] = x_ub;
        data[1][0] = y_lb; data[1][1] = y_ub;

        bool valid = true;
        if((!m_U.empty() && m_U[0][position[0][0]][position[1][0]]==m_U_Fill_Value)
           || (!m_V.empty() && m_V[0][position[0][0]][position[1][0]]==m_U_Fill_Value))
            valid = false;
        leaf.first->set_node_val(data, valid);
    }
}

std::vector<std::array<int, 2>> get_empty_position(){
    std::vector<std::array<int, 2>> target;
    target.push_back(std::array<int, 2>({std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}));
    target.push_back(std::array<int, 2>({std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}));
    return target;
}

void LambertGrid::compute_grid_proj(NcFile &dataFile){

    /// **************************************************************
    /// *************************** NcData ***************************

//    NcFile dataFile(file_name, NcFile::read);

    // LambertGrid Size
    size_t nj_u_size = dataFile.getDim("nj_u").getSize();
    size_t ni_u_size = dataFile.getDim("ni_u").getSize();
    const size_t nij_u_size = nj_u_size*ni_u_size;
    size_t nj_v_size = dataFile.getDim("nj_v").getSize();
    size_t ni_v_size = dataFile.getDim("ni_v").getSize();
    const size_t nij_v_size = nj_v_size*ni_v_size;

    // Longitude/Longitude for U
    double *longitude_u = new double[nij_u_size]();
    double *latitude_u = new double[nij_u_size]();
    load_raw_data(dataFile, "longitude_u", longitude_u, nij_u_size);
    load_raw_data(dataFile, "latitude_u", latitude_u, nij_u_size);

    double *longitude_v = new double[nij_v_size]();
    double *latitude_v = new double[nij_v_size]();
    load_raw_data(dataFile, "longitude_v", longitude_v, nij_v_size);
    load_raw_data(dataFile, "latitude_v", latitude_v, nij_v_size);

    /// **************************************************************
    /// *************************** Proj *****************************

    // Init proj
    projPJ pj_lambert, pj_latlong;
    if (!(pj_lambert = pj_init_plus("+init=epsg:2154"))){
      cout << "[Lambert_node] Error Lambert" << endl;
      exit(1);
    }
    if (!(pj_latlong = pj_init_plus("+init=epsg:4326"))){
      cout << "[Lambert_node] Error LatLong" << endl;
      exit(1);
    }

    pj_transform(pj_latlong, pj_lambert, nij_u_size, 1, longitude_u, latitude_u, nullptr);
    pj_transform(pj_latlong, pj_lambert, nij_v_size, 1, longitude_v, latitude_v, nullptr);
    pj_free(pj_lambert);
    pj_free(pj_latlong);

    fill_vector(m_U_X, m_U_Y, longitude_u, latitude_u, ni_u_size, nj_u_size);
    fill_vector(m_V_X, m_V_Y, longitude_v, latitude_v, ni_v_size, nj_v_size);

    delete[] latitude_u;
    delete[] longitude_u;
    delete[] latitude_v;
    delete[] longitude_v;

    /// **************************************************************
    /// *************************** Build tree ***********************

    // Create Tree
    m_position_U.push_back(std::array<int, 2>({0, (int)ni_u_size-1}));
    m_position_U.push_back(std::array<int, 2>({0, (int)nj_u_size-1}));
    std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > > leaf_list_U;
    m_dataSet_U = new DataSetNode<double, 2>(m_position_U, leaf_list_U);

    m_position_V.push_back(std::array<int, 2>({0, (int)ni_v_size-1}));
    m_position_V.push_back(std::array<int, 2>({0, (int)nj_v_size-1}));
    std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > > leaf_list_V;
    m_dataSet_V = new DataSetNode<double, 2>(m_position_V, leaf_list_V);

    // Fill the leaf
    fill_leaf(leaf_list_U, m_U_X, m_U_Y);
    fill_leaf(leaf_list_V, m_U_X, m_U_Y);

    // Fill the tree
    m_dataSet_U->fill_tree();
    m_dataSet_V->fill_tree();
}

LambertGrid::LambertGrid(const std::string &file_xml){
    pt::ptree tree;
    cout << " READ file_xml = " << file_xml << endl;
    pt::read_xml(file_xml, tree);

    string directory = tree.get<string>("PREVIMER.<xmlattr>.directory");
    cout << " DATA Directory = " << directory<< endl;
    int file_id = 0;
    bool first_read = true;

    BOOST_FOREACH(pt::ptree::value_type &v, tree.get_child("PREVIMER")){
        if(v.first == "file"){
            string file_name = v.second.get<string>("");
            if(!file_name.empty()){
                cout << " " << file_id << " " << file_name << endl;
                file_id++;

                NcFile dataFile(directory+file_name, NcFile::read);

                NcVar u_var=dataFile.getVar("U");
                NcVar v_var=dataFile.getVar("V");

                // Global parameters
                if(first_read){
                    u_var.getAtt("scale_factor").getValues(&m_U_scale_factor);
                    u_var.getAtt("add_offset").getValues(&m_U_add_offset);
                    u_var.getAtt("_FillValue").getValues(&m_U_Fill_Value);
                    v_var.getAtt("scale_factor").getValues(&m_V_scale_factor);
                    v_var.getAtt("add_offset").getValues(&m_V_add_offset);
                    v_var.getAtt("_FillValue").getValues(&m_V_Fill_Value);
                }

                // ******* Size U,V ******
                size_t nj_u = dataFile.getDim("nj_u").getSize();
                size_t ni_u = dataFile.getDim("ni_u").getSize();
                size_t nj_v = dataFile.getDim("nj_v").getSize();
                size_t ni_v = dataFile.getDim("ni_v").getSize();

                // ******* DATA U,V ******
                short int *raw_u = new short int[nj_u*ni_u];
                short int *raw_v = new short int[nj_v*ni_v];
                u_var.getVar(raw_u);
                v_var.getVar(raw_v);

                vector<vector<short int>> tab_u, tab_v;

                for(size_t i=0; i<ni_u; i++){
                    vector<short int> line_u;
                    for(size_t j=0; j<nj_u; j++){
                        line_u.push_back(raw_u[j*ni_u+i]); // y, x tab
                    }
                    tab_u.push_back(line_u);
                }

                for(size_t i=0; i<ni_v; i++){
                    vector<short int> line_v;
                    for(size_t j=0; j<nj_v; j++){
                        line_v.push_back(raw_v[j*ni_v+i]); // y, x tab
                    }
                    tab_v.push_back(line_v);
                }

                delete[] raw_u;
                delete[] raw_v;

                m_U.push_back(tab_u);
                m_V.push_back(tab_v);

                if(first_read){
                    compute_grid_proj(dataFile);
                    first_read = false;
                }
            }
        }
    }
}

void LambertGrid::eval(const double &x, const double &y, const double &t, double &u, double &v) const{
    /// **************************************************************
    /// *************************** Tests ****************************
    // Test
    std::vector<std::array<int, 2>> target = get_empty_position();

    array<array<double, 2>, 2> data;
    data[0][0] = 102403; data[0][1] = 102490;
    data[1][0] = 6848055; data[1][1] = 6848100;
//    data[0][0] = 49851; data[0][1] = 49853;
//    data[1][0] = 6728480; data[1][1] = 6728490;

//    array<array<double, 2>, 2> data;
//    data[0][0] = 10.0; data[0][1] = 11.0;
//    data[1][0] = 1.0; data[1][1] = 2.0;

    m_dataSet_U->eval_invert(target, m_position_U, data);

    cout << m_dataSet_U->get_number_node() << endl;
    cout << m_dataSet_U->get_number_leaf() << endl;

    cout << target[0][0] << " " << target[0][1] << endl;
    cout << target[1][0] << " " << target[1][1] << endl;

}

LambertGrid::~LambertGrid(){
    if(m_dataSet_U != nullptr)
        delete(m_dataSet_U);
    if(m_dataSet_V != nullptr)
        delete(m_dataSet_V);
}

}
