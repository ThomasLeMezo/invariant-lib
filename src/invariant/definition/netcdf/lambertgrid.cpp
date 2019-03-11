#include "lambertgrid.h"
#include <netcdf>
#include <omp.h>
#include <proj.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <math.h>

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

  double scale_factor=1.0;
  double add_offset = 0.0;
  var.getAtt("scale_factor").getValues(&scale_factor);
  var.getAtt("add_offset").getValues(&add_offset);

  short int *raw = new short int[data_size];
  var.getVar(raw);

  for(size_t i=0; i<data_size; i++)
    data[i] = proj_torad(scale_factor*raw[i]+add_offset);

  delete[] raw;
}

void load_raw_data_double(const NcFile &dataFile, const string &var_name, double *data, const size_t &data_size){
  NcVar var=dataFile.getVar(var_name);

  double *raw = new double[data_size];
  var.getVar(raw);

  for(size_t i=0; i<data_size; i++)
    data[i] = proj_torad(raw[i]);

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

void fill_vector(std::vector<std::vector<short int>> &X, short int *dataX, const size_t &ni, const size_t &nj){
  for(size_t i=0; i<ni; i++){
    X.push_back(vector<short int>());
    for(size_t j=0; j<nj; j++){
      X[i].push_back(dataX[j*ni+i]);
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

std::array<std::array<double, 2>, 2> get_empty_data(){
  std::array<std::array<double, 2>, 2> data;
  data[0][0] = std::numeric_limits<double>::max();
  data[0][1] = std::numeric_limits<double>::min();
  data[1][0] = std::numeric_limits<double>::max();
  data[1][1] = std::numeric_limits<double>::min();
  return data;
}

double find_max_distance(const std::vector<std::vector<double>> &data){
  double d = 0.0;
  //    size_t i_save, j_save;

  for(size_t i=0; i<data.size()-1; i++){
    for(size_t j=0; j<data[0].size()-1; j++){
      double d_test = max(abs(data[i][j+1] - data[i][j]), abs(data[i+1][j] - data[i][j]));
      if(d_test>d){
        d=d_test;
        //                i_save = i; j_save=j;
      }
    }
  }
  //    cout << "max distance = "<< d << " (" << i_save << ", " << j_save << ")" << endl;

  return d;
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
  size_t nj_size = dataFile.getDim("nj").getSize();
  size_t ni_size = dataFile.getDim("ni").getSize();
  const size_t nij_size = nj_size*ni_size;

  // Longitude/Longitude for U
  double *longitude_u = new double[nij_u_size]();
  double *latitude_u = new double[nij_u_size]();
  load_raw_data(dataFile, "longitude_u", longitude_u, nij_u_size);
  load_raw_data(dataFile, "latitude_u", latitude_u, nij_u_size);

  double *longitude_v = new double[nij_v_size]();
  double *latitude_v = new double[nij_v_size]();
  load_raw_data(dataFile, "longitude_v", longitude_v, nij_v_size);
  load_raw_data(dataFile, "latitude_v", latitude_v, nij_v_size);

  double *longitude = new double[nij_size]();
  double *latitude = new double[nij_size]();
  short int *H0 = new short int[nij_size]();
  load_raw_data_double(dataFile, "longitude", longitude, nij_size);
  load_raw_data_double(dataFile, "latitude", latitude, nij_size);

  NcVar var_H0=dataFile.getVar("H0");
  var_H0.getAtt("_FillValue").getValues(&m_H0_Fill_Value);
  var_H0.getVar(H0);

  /// **************************************************************
  /// *************************** Proj *****************************

  // Convergence angle correction

  double lambda_0 = proj_torad(3.0);
  double lambda_mean = longitude[(nj_size/2)*ni_size+ni_size/2];
  double latitude_mean = latitude[(nj_size/2)*ni_size+ni_size/2];

  m_convergence_angle = (lambda_mean-lambda_0)*sin(latitude_mean);
  m_conv_cos = cos(m_convergence_angle);
  m_conv_sin = sin(m_convergence_angle);
//      cout << "lambda_mean = " << lambda_mean << endl;
//      cout << "lambda_0 = " << lambda_0 << endl;
//      cout << "latitude_mean = " << latitude_mean << endl;
//      cout << "m_convergence_angle = " << m_convergence_angle << endl;
  cout << "Convergence angle = " << m_convergence_angle << endl;

  // *******************************

  // Init proj
  m_C = proj_context_create();

  m_pj_lambert2latlong = proj_create(m_C, "+proj=lcc +lat_1=49 +lat_2=44 +lat_0=46.5 +lon_0=3 +x_0=700000 +y_0=6600000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs ");
  //    PJ *pj_lambert2latlong = proj_create(C, "+proj=epsg:2154");
  //    PJ *pj_lambert2latlong = proj_create_crs_to_crs(C, "epsg:4326", "epsg:2154", 0);
  //    PJ *pj_lambert2latlong = proj_create(C, "+init=urn:ogc:def:crs:epsg::2154");
  //    PJ *pj_latlong = proj_create(C, "+init=epsg:4326");

  if(m_pj_lambert2latlong==0){
    cout << "[Lambert_node] Error Lambert " << proj_errno_string(proj_errno(m_pj_lambert2latlong)) << endl;
    exit(1);
  }

  PJ_COORD *coord_u = new PJ_COORD[nij_u_size];
  for(size_t i=0; i<nij_u_size; i++){
    coord_u[i].lp.lam = longitude_u[i];
    coord_u[i].lp.phi = latitude_u[i];
  }

  PJ_COORD *coord_v = new PJ_COORD[nij_v_size];
  for(size_t i=0; i<nij_v_size; i++){
    coord_v[i].lp.lam = longitude_v[i];
    coord_v[i].lp.phi = latitude_v[i];
  }

  PJ_COORD *coord = new PJ_COORD[nij_size];
  for(size_t i=0; i<nij_size; i++){
    coord[i].lp.lam = longitude[i];
    coord[i].lp.phi = latitude[i];
  }


  proj_trans_array(m_pj_lambert2latlong, PJ_FWD, nij_size, coord);
  proj_trans_array(m_pj_lambert2latlong, PJ_FWD, nij_u_size, coord_u);
  proj_trans_array(m_pj_lambert2latlong, PJ_FWD, nij_v_size, coord_v);

  for(size_t i=0; i<nij_u_size; i++){
    longitude_u[i] = coord_u[i].xy.x;
    latitude_u[i] = coord_u[i].xy.y;
  }

  for(size_t i=0; i<nij_v_size; i++){
    longitude_v[i] = coord_v[i].xy.x;
    latitude_v[i] = coord_v[i].xy.y;
  }

  for(size_t i=0; i<nij_size; i++){
    longitude[i] = coord[i].xy.x;
    latitude[i] = coord[i].xy.y;
  }

  delete[] coord_u;
  delete[] coord_v;
  delete[] coord;

  fill_vector(m_U_X, m_U_Y, longitude_u, latitude_u, ni_u_size, nj_u_size);
  fill_vector(m_V_X, m_V_Y, longitude_v, latitude_v, ni_v_size, nj_v_size);
  fill_vector(m_X, m_Y, longitude, latitude, ni_size, nj_size);
  fill_vector(m_H0, H0, ni_size, nj_size);

  delete[] latitude_u;
  delete[] longitude_u;
  delete[] latitude_v;
  delete[] longitude_v;
  delete[] latitude;
  delete[] longitude;
  delete[] H0;

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

  /// **************************************************************
  /// *************************** Find distances *******************
  m_distance_max_U_X = find_max_distance(m_U_X);
  m_distance_max_U_Y = find_max_distance(m_U_Y);
  m_distance_max_V_X = find_max_distance(m_V_X);
  m_distance_max_V_Y = find_max_distance(m_V_Y);

  m_distance_max_U = 1.1*max(m_distance_max_U_X, m_distance_max_U_Y);
  m_distance_max_V = 1.1*max(m_distance_max_V_X, m_distance_max_V_Y);
  //    m_distance_max_U = sqrt(m_distance_max_U_X*m_distance_max_U_X + m_distance_max_U_Y*m_distance_max_U_Y);
  //    m_distance_max_V = sqrt(m_distance_max_V_X*m_distance_max_V_X + m_distance_max_V_Y*m_distance_max_V_Y);
}

LambertGrid::LambertGrid(const std::string &file_xml){
  // Variable Init
  int max_threads = omp_get_max_threads();
  for(int i=0; i<max_threads; i++){
    m_last_data_U_return.push_back(array<array<double, 2>, 2>());
    m_last_data_V_return.push_back(array<array<double, 2>, 2>());
    m_last_target_U.push_back(std::vector<std::array<int, 2>>());
    m_last_target_V.push_back(std::vector<std::array<int, 2>>());
  }

  // Load files
  pt::ptree tree;
  cout << " READ file_xml = " << file_xml << endl;
  pt::read_xml(file_xml, tree);

  string directory = tree.get<string>("PREVIMER.<xmlattr>.directory");
  //    cout << " DATA Directory = " << directory<< endl;
  int file_id = 0;
  bool first_read = true;

  BOOST_FOREACH(pt::ptree::value_type &v, tree.get_child("PREVIMER")){
    if(v.first == "file"){
      string file_name = v.second.get<string>("");
      if(!file_name.empty()){
        //                cout << " " << file_id << " " << file_name << endl;
        file_id++;

        NcFile dataFile(directory+file_name, NcFile::read);

        NcVar u_var=dataFile.getVar("U");
        NcVar v_var=dataFile.getVar("V");
        NcVar time_var = dataFile.getVar("time");

        /// ******* Global parameters ******
        if(first_read){
          u_var.getAtt("scale_factor").getValues(&m_U_scale_factor);
          u_var.getAtt("add_offset").getValues(&m_U_add_offset);
          u_var.getAtt("_FillValue").getValues(&m_U_Fill_Value);
          v_var.getAtt("scale_factor").getValues(&m_V_scale_factor);
          v_var.getAtt("add_offset").getValues(&m_V_add_offset);
          v_var.getAtt("_FillValue").getValues(&m_V_Fill_Value);
        }

        /// ******* Time **********
        double t;
        time_var.getVar(&t);
        m_time.push_back(t-2208988800); // Convert to POSIX Time

        /// ******* DATA U,V ******
        // Size U,V
        size_t nj_u = dataFile.getDim("nj_u").getSize();
        size_t ni_u = dataFile.getDim("ni_u").getSize();
        size_t nj_v = dataFile.getDim("nj_v").getSize();
        size_t ni_v = dataFile.getDim("ni_v").getSize();

        // Data
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

        /// ******* Grid Lambert Projection ******
        if(first_read){
          compute_grid_proj(dataFile);
          first_read = false;
        }
      }
    }
  }

  // Apply correction to data?

  cout << "Number files read = " << file_id << endl;

  //    for(double &t:m_time)
  //      cout << t << endl;

  if(m_time.size()>1){
    m_time_min = m_time[0];
    m_time_max = m_time[m_time.size()-1];
    m_time_dt = m_time[1]-m_time[0];
  }
}

size_t LambertGrid::get_time_grid(const double &t) const{
  if(t<m_time_min)
    return 0;
  else if(t>m_time_max)
    return m_time.size()-1;
  else
    return (size_t) max(0, (int)(std::upper_bound(m_time.begin(), m_time.end(), t) - m_time.begin()-1));
}

bool sort_pair(const std::pair<double, std::array<int, 2>> &d1, const std::pair<double, std::array<int, 2>> &d2){return (d1.first<d2.first);}

double compute_ponderation(const std::vector<std::pair<double, std::array<int, 2>>> &map_distance,
                           const std::vector<std::vector<std::vector<short int>>>& data,
                           const size_t &t,
                           const double &scale_factor,
                           const double &add_offset){
  double sum_coeff = 0.0;
  double u = 0.0;
  for(size_t i=0; i<min((size_t)4, map_distance.size()); i++){
    // Liszka ponderation
    double coeff = 1.0/sqrt(pow(map_distance[i].first, 2)+0.01);
    u += coeff*((data[t][map_distance[i].second[0]][map_distance[i].second[1]])*scale_factor+add_offset);
    sum_coeff += coeff;
  }
  if(sum_coeff!=0.0)
    u /= sum_coeff;
  return u;
}

int LambertGrid::eval(const double &x, const double &y, const double &t, double &u, double &v) const{
  // Find corresponding Data
  bool data_found = true;

  const size_t t1 = get_time_grid(t);
  if(abs(t-m_time[t1])>m_time_dt*2.0){
    cout << "invalid time" << endl;
    return false;
  }

  int thread_id = omp_get_thread_num();

  // Area to search
  array<array<double, 2>, 2> data_U;
  data_U[0][0] = x-m_distance_max_U_X; data_U[0][1] = x+m_distance_max_U_X;
  data_U[1][0] = y-m_distance_max_U_Y; data_U[1][1] = y+m_distance_max_U_Y;
  array<array<double, 2>, 2> data_V;
  data_V[0][0] = x-m_distance_max_V_X; data_V[0][1] = x+m_distance_max_V_X;
  data_V[1][0] = y-m_distance_max_V_Y; data_V[1][1] = y+m_distance_max_V_Y;

  std::vector<std::array<int, 2>> target_U = get_empty_position();
  std::vector<std::array<int, 2>> target_V = get_empty_position();

  if(m_dataSet_U->is_subset_data(data_U, m_last_data_U_return[thread_id]))
    target_U = m_last_target_U[thread_id];
  else{
    array<array<double, 2>, 2> data_U_return = get_empty_data();
    data_found &= m_dataSet_U->eval_invert(target_U, m_position_U, data_U, data_U_return);
    m_last_target_U[thread_id] = target_U;
    m_last_data_U_return[thread_id] = data_U_return;
  }

  if(m_dataSet_V->is_subset_data(data_V, m_last_data_V_return[thread_id]))
    target_V = m_last_target_V[thread_id];
  else{
    array<array<double, 2>, 2> data_V_return = get_empty_data();
    data_found &= m_dataSet_V->eval_invert(target_V, m_position_V, data_V, data_V_return);
    m_last_target_V[thread_id] = target_V;
    m_last_data_V_return[thread_id] = data_V_return;
  }

  std::vector<std::pair<double, std::array<int, 2>>> vector_distance_U;
  std::vector<std::pair<double, std::array<int, 2>>> vector_distance_V;

  if(data_found){
    for(int i=target_U[0][0]; i<=target_U[0][1]; i++){
      for(int j=target_U[1][0]; j<=target_U[1][1]; j++){
        double d = sqrt(pow(m_U_X[i][j]-x, 2)+pow(m_U_Y[i][j]-y, 2));
        if(d<m_distance_max_U)
          vector_distance_U.push_back(make_pair(d, array<int, 2>({i, j})));
      }
    }

    for(int i=target_V[0][0]; i<=target_V[0][1]; i++){
      for(int j=target_V[1][0]; j<=target_V[1][1]; j++){
        double d = sqrt(pow(m_V_X[i][j]-x, 2)+pow(m_V_Y[i][j]-y, 2));
        if(d<m_distance_max_V)
          vector_distance_V.push_back(make_pair(d, array<int, 2>({i, j})));
      }
    }

    // Test if the point is inside none valid area
    if(vector_distance_U.empty() || vector_distance_V.empty()){
      u=0.0;
      v=0.0;
      return -1;
    }

    // Sort distances
    std::sort(vector_distance_U.begin(), vector_distance_U.end(), sort_pair);
    std::sort(vector_distance_V.begin(), vector_distance_V.end(), sort_pair);

    // Compute Current values : take 4 closest
    double u_t1 = compute_ponderation(vector_distance_U, m_U, t1, m_U_scale_factor, m_U_add_offset);
    double v_t1 = compute_ponderation(vector_distance_V, m_V, t1, m_V_scale_factor, m_V_add_offset);

    const size_t t2 = t1+1;
    if(t2<m_time.size() && (t-m_time[t1]>0)){ // Check if t is between m_time[t1] and m_time[t2]
      double u_t2 = u_t1;
      double v_t2 = v_t1;
      const double d_t1_t = abs(m_time[t1]-t);
      const double d_t2_t = abs(m_time[t2]-t);
      u_t2 = compute_ponderation(vector_distance_U, m_U, t2, m_U_scale_factor, m_U_add_offset);
      v_t2 = compute_ponderation(vector_distance_V, m_V, t2, m_V_scale_factor, m_V_add_offset);

      const double coeff1 = (1/sqrt(d_t1_t*d_t1_t+0.01));
      const double coeff2 = (1/sqrt(d_t2_t*d_t2_t+0.01));
      const double sum_coeff = coeff1+coeff2;
      u = (coeff1*u_t1+coeff2*u_t2)/sum_coeff;
      v = (coeff1*v_t1+coeff2*v_t2)/sum_coeff;
    }
    else{
      u = u_t1;
      v = v_t1;
    }

    // Convergence compensation
    PJ_COORD coord;
    coord.xy.x = x;
    coord.xy.y = y;
    PJ_COORD coord_new = proj_trans(m_pj_lambert2latlong, PJ_INV, coord);

    const double gamma = (coord_new.lp.lam-m_lambda_0)*sin(coord_new.lp.phi);

    double u_new = u*cos(gamma) + v*sin(gamma);
    double v_new = v*cos(gamma) + u*sin(gamma);
    u=u_new;
    v=v_new;

    return 0;
  }
  else{
    u=0.0;
    v=0.0;
    return -2;
  }
}

LambertGrid::~LambertGrid(){
  if(m_dataSet_U != nullptr)
    delete(m_dataSet_U);
  if(m_dataSet_V != nullptr)
    delete(m_dataSet_V);
  proj_destroy(m_pj_lambert2latlong);
  proj_context_destroy(m_C);
}

bool LambertGrid::rk2(const double &t_init, const double &duration,
                      const double &x_init, const double &y_init,
                      const double &dt,
                      array<vector<double>,3> &result,
                      bool break_on_error) const{
  double x_tmp = x_init;
  double y_tmp = y_init;
  bool valid = true;
  for(double t=t_init; t<(t_init+duration); t+=dt){
    // RK2 scheme
    double u, v;
    this->eval(x_tmp, y_tmp, t, u, v);
    double x_r = x_tmp + (dt/2.0)*u;
    double y_r = y_tmp + (dt/2.0)*v;

    if(!this->eval(x_r, y_r, t+(dt/2.0), u, v))
      valid = false;

    x_tmp+=u*dt;
    y_tmp+=v*dt;

    result[0].push_back(t);
    result[1].push_back(x_tmp);
    result[2].push_back(y_tmp);

    if(break_on_error && !valid)
      break;
  }
  return valid;
}

}
