#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>

using namespace std;
using namespace invariant;


int main(int argc, char *argv[]){

    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");

    vibes::beginDrawing();
    vibes::newFigure("Ouessant");

    // Draw Ouessant

    vector<double> ouessant_x({99115,100427,107277,103975, 99115});
    vector<double> ouessant_y({6847481,6845602,6848309,6851432, 6847481});
    vibes::drawLine(ouessant_x, ouessant_y, "red");
//    vibes::axisAuto("Ouessant");

    // Compute trajectories

    double x_init, y_init, t_init, u, v;
    t_init = 3708553500;
    double dt = 1.0;

//    x_init = 96908;
//    y_init = 6846398;
    x_init = 97339+250*3;
    y_init = 6848741;
    double t_start = 0;
    size_t k=0;

//    g.eval(97554, 6849273, t_init, u, v);
//    cout << u << endl;
//    cout << v << endl;

    for(double x=x_init; x<x_init+250; x+=50.0){
        for(double y=y_init; y<y_init+250; y+=50.0){
            vector<double> x_position, y_position;
            double x_tmp = x;
            double y_tmp = y;
            cout << "> Compute Traj " << k++ << endl;
            for(double t=t_init+t_start; t<t_init+3600*24; t+=dt){
                if(!g.eval(x_tmp, y_tmp, t, u, v))
                    break;
                x_tmp+=u*dt;
                y_tmp+=v*dt;
                x_position.push_back(x_tmp);
                y_position.push_back(y_tmp);
            }
            vibes::drawLine(x_position, y_position);
        }
    }

    vibes::axisAuto("Ouessant");
}
