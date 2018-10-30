#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>

#include "vtkMaze3D.h"

#include <omp.h>

using namespace std;
using namespace invariant;

int main(int argc, char *argv[]){
    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files_rade.xml");

    double time_start = omp_get_wtime();

    // Compute trajectories
    double x_init, y_init, t_init;
    double dt = 1.0;

    t_init = stoll(argv[1]);
    x_init = stoll(argv[2]);
    y_init = stoll(argv[3]);

    cout << "t_init = " << std::fixed << t_init << endl;
    cout << "x_init = " << std::fixed << x_init << endl;
    cout << "y_init = " << std::fixed << y_init << endl;

    double t_max = g.get_time_max();
    double t_min = g.get_time_min();
    cout << "Tmin = " << std::fixed << t_min << " Tmax = " << t_max << endl;

    vector<array<double, 3>> init_conditions;
    for(double x=-1; x<=1; x++){
        for(double y=-1; y<=1; y++){
            for(double t=-1; t<=1; t++){
                init_conditions.push_back(array<double, 3>{x_init+25.0*x, y_init+25*y, t_init+t*5*60.});
            }
        }
    }

    vector<array<vector<double>,3>> result;

    const size_t nb_conditions = init_conditions.size();
#pragma omp parallel for
    for(size_t k=0; k<nb_conditions; k++){
            double x_tmp = init_conditions[k][0];
            double y_tmp = init_conditions[k][1];
            double t_tmp = init_conditions[k][2];
//            bool valid = true;
//#pragma omp critical
//            {
//            cout << "> Compute Traj " << k << endl;
//            }

            array<vector<double>,3> sim_result;
            g.rk2(t_tmp,t_max, x_tmp, y_tmp, dt,sim_result);

#pragma omp critical
            {
//              cout << "--> simu.size = " << sim_result[0].size() << endl;
                result.push_back(sim_result);
            }
        }

    ofstream myfile;
    myfile.open ("/home/lemezoth/seabot_forcast.txt");
    for(size_t i=0; i<result[0][0].size(); i++){
      myfile << std::fixed << result[0][2][i] << " " << result[0][0][i] << " " << result[0][1][i] << endl;
    }
    myfile.close();

    cout << "TIME = " << omp_get_wtime() - time_start << endl;
}
