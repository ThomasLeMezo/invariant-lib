#include "lambertgrid.h"
#include <netcdf>
#include <omp.h>
#include <proj_api.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

namespace invariant {

LambertGrid::LambertGrid(const string &file_name){

    /// **************************************************************
    /// *************************** NcData ***************************

    NcFile dataFile(file_name, NcFile::read);

    // LambertGrid Size
    size_t nj_u_size = dataFile.getDim("nj_u").getSize();
    size_t ni_u_size = dataFile.getDim("ni_u").getSize();
    size_t nij_u_size = nj_u_size*ni_u_size;

    // Longitude for U
    NcVar longitude_u_var=dataFile.getVar("longitude_u");

    double longitude_u_scale_factor,longitude_u_add_offset;
    short int longitude_u_fill_value;

    longitude_u_var.getAtt("scale_factor").getValues(&longitude_u_scale_factor);
    longitude_u_var.getAtt("add_offset").getValues(&longitude_u_add_offset);
    longitude_u_var.getAtt("_FillValue").getValues(&longitude_u_fill_value);

    short int *longitude_u_raw = new short int[nij_u_size];
    longitude_u_var.getVar(longitude_u_raw);

    double *longitude_u = new double[nij_u_size];
    for(size_t i=0; i<nij_u_size; i++)
        longitude_u[i] = (longitude_u_scale_factor*longitude_u_raw[i]+longitude_u_add_offset)*DEG_TO_RAD;

    // Latitude for U
    NcVar latitude_u_var=dataFile.getVar("latitude_u");

    double latitude_u_scale_factor,latitude_u_add_offset;
    short int latitude_u_fill_value;

    latitude_u_var.getAtt("scale_factor").getValues(&latitude_u_scale_factor);
    latitude_u_var.getAtt("add_offset").getValues(&latitude_u_add_offset);
    latitude_u_var.getAtt("_FillValue").getValues(&latitude_u_fill_value);

    short int *latitude_u_raw = new short int[nj_u_size*ni_u_size];
    latitude_u_var.getVar(latitude_u_raw);

    double *latitude_u = new double[nij_u_size];
    for(size_t i=0; i<nij_u_size; i++)
        latitude_u[i] = (latitude_u_scale_factor*latitude_u_raw[i]+latitude_u_add_offset)*DEG_TO_RAD;

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

    vector<vector<double>> X, Y;
    for(size_t i=0; i<ni_u_size; i++){
        m_X.push_back(vector<double>());
        m_Y.push_back(vector<double>());
        for(size_t j=0; j<nj_u_size; j++){
            m_X[i].push_back(longitude_u[j*ni_u_size+i]);
            m_Y[i].push_back(latitude_u[j*ni_u_size+i]);
        }
    }

    delete[] longitude_u_raw;
    delete[] latitude_u_raw;
    delete[] latitude_u;
    delete[] longitude_u;

    pj_free(pj_lambert);
    pj_free(pj_latlong);

    /// **************************************************************
    /// *************************** Build tree ***********************

    // Create Tree
    std::vector<std::array<int, 2>> position;
    position.push_back(std::array<int, 2>({0, (int)ni_u_size-1}));
    position.push_back(std::array<int, 2>({0, (int)nj_u_size-1}));
    std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > > leaf_list;
    m_dataSet_U = new DataSetNode<double, 2>(position, leaf_list);

    // Fill the leaf
    for(std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > &leaf:leaf_list){
        vector<array<int, 2>> position = leaf.second;
        array<array<double, 2>, 2> data;
        double x_center = m_X[position[0][0]][position[1][0]];
        double x_ub, x_lb;
        if(position[0][0]+1<(int)ni_u_size)
            x_ub = x_center + ceil(abs(m_X[position[0][0]+1][position[1][0]]-x_center)/2.0);
        else
            x_ub = x_center;

        if(position[0][0]-1>=0)
            x_lb = x_center - ceil(abs(x_center-m_X[position[0][0]-1][position[1][0]])/2.0);
        else
            x_lb = x_center;

        double y_center = m_Y[position[0][0]][position[1][0]];
        double y_ub, y_lb;
        if(position[1][0]+1<(int)nj_u_size)
            y_ub = y_center + ceil(abs(m_Y[position[0][0]][position[1][0]+1]-y_center)/2.0);
        else
            y_ub = y_center;

        if(position[1][0]-1>=0)
            y_lb = y_center - ceil(abs(y_center-m_Y[position[0][0]][position[1][0]-1])/2.0);
        else
            y_lb = y_center;

        data[0][0] = x_lb; data[0][1] = x_ub;
        data[1][0] = y_lb; data[1][1] = y_ub;
        leaf.first->set_node_val(data, true);
    }

    // Fill the tree
    m_dataSet_U->fill_tree();

    std::vector<std::array<int, 2>> target;
    target.push_back(std::array<int, 2>({1000, -1}));
    target.push_back(std::array<int, 2>({1000, -1}));

    array<array<double, 2>, 2> data;
    data[0][0] = 102403; data[0][1] = 102490;
    data[1][0] = 6848055; data[1][1] = 6848100;
//    data[0][0] = 49851; data[0][1] = 49853;
//    data[1][0] = 6728480; data[1][1] = 6728490;

    cout << m_X[0][0] << endl;
    cout << m_Y[0][0] << endl;

//    array<array<double, 2>, 2> data;
//    data[0][0] = 10.0; data[0][1] = 11.0;
//    data[1][0] = 1.0; data[1][1] = 2.0;

    m_dataSet_U->eval_invert(target, position, data);

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
