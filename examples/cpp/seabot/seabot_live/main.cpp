#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>
#include <algorithm>

#include "vtkMaze3D.h"

#include <omp.h>

using namespace std;
using namespace invariant;

bool array_sort (array<double, 3> i,array<double, 3> j) { return (i[2]<j[2]); }

int main(int argc, char *argv[]){
    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files_rade.xml");

    double time_start = omp_get_wtime();

    // Compute trajectories
    double x_init, y_init, t_init;
    double dt = 1.0;

    t_init = stoll(argv[1]);
    x_init = stoll(argv[2]);
    y_init = stoll(argv[3]);

//    t_init = 1541394000.000000;
//    x_init = 149557.000000;
//    y_init = 6827729.000000;

    cout << "t_init = " << std::fixed << t_init << endl;
    cout << "x_init = " << std::fixed << x_init << endl;
    cout << "y_init = " << std::fixed << y_init << endl;

    double t_max = g.get_time_max();
    double t_min = g.get_time_min();
    double duration = min(t_max - t_min, 4.*3600); // To Change ?
    cout << "Tmin = " << std::fixed << t_min << " Tmax = " << t_max << " duration = " << duration << endl;

    vector<array<double, 4>> init_conditions;
    for(double x=-1; x<=1; x++){
        for(double y=-1; y<=1; y++){
            for(double t=-1; t<=1; t++){
                bool center = false;
                if(x==0 && y==0 && t==0)
                    center = true;
                init_conditions.push_back(array<double, 4>{t_init+t*5*60., x_init+10.0*x, y_init+10*y, (double)center});
            }
        }
    }

    vector<array<vector<double>,3>> result_polygon;
    array<vector<double>,3> result_center;

    const size_t nb_conditions = init_conditions.size();
#pragma omp parallel for
    for(size_t k=0; k<nb_conditions; k++){
            double t_tmp = init_conditions[k][0];
            double x_tmp = init_conditions[k][1];
            double y_tmp = init_conditions[k][2];
            bool is_center = (bool)init_conditions[k][3];

            array<vector<double>,3> sim_result;
            g.rk2(t_tmp,duration, x_tmp, y_tmp, dt,sim_result, false);

#pragma omp critical
            {
                if(is_center)
                    result_center = sim_result;
                result_polygon.push_back(sim_result);
            }
        }

    ofstream myfile;
    myfile.open ("/home/lemezoth/iridium/seabot_forcast.txt");
    for(size_t i=0; i<result_center[0].size(); i+=15){
      myfile << std::fixed << result_center[0][i] << " " << result_center[1][i] << " " << result_center[2][i] << endl;
    }
    myfile.close();

    ofstream myfile_polygon;
    myfile_polygon.open ("/home/lemezoth/iridium/seabot_forcast_polygon.txt");
    for(size_t i=0; i<result_polygon[0][0].size(); i+=15){
      myfile_polygon << std::fixed << result_polygon[0][0][i];

      vector<array<double, 3>> tmp_points;
      for(size_t j=0; j<result_polygon.size(); j++)
          tmp_points.push_back(array<double, 3>{result_polygon[j][1][i], result_polygon[j][2][i], 0.0});
      // Compute mean position
      double x_mean=0.0, y_mean=0.0;
      for(size_t j=0; j<tmp_points.size(); j++){
        x_mean += tmp_points[j][0];
        y_mean += tmp_points[j][1];
      }
      x_mean /= tmp_points.size();
      y_mean /= tmp_points.size();
      // Compute angle
      for(size_t j=0; j<tmp_points.size(); j++){
          tmp_points[j][2] = atan2(tmp_points[j][1]-y_mean, tmp_points[j][0]-x_mean);
      }
      // Sort angle
      std::sort(tmp_points.begin(), tmp_points.end(), array_sort);

      for(size_t j=0; j<tmp_points.size(); j++){
        myfile_polygon << " " << tmp_points[j][0] << " " << tmp_points[j][1];
      }
      myfile_polygon << endl;
    }
    myfile.close();

    cout << "TIME = " << omp_get_wtime() - time_start << endl;
}
