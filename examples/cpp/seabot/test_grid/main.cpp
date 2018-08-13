#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>

using namespace std;
using namespace invariant;


int main(int argc, char *argv[]){

    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");
    double x, y, t_init, u, v;
    x = 96908;
    y = 6846398;
    t_init = 3708553500;
    double dt = 1.0;

    vector<double> x_position, y_position;

    for(double t=t_init; t<t_init+3600*10; t+=dt){
        g.eval(x, y, t, u, v);
        x+=u*dt;
        y+=v*dt;
        x_position.push_back(x);
        y_position.push_back(y);
    }

    vibes::beginDrawing();
    vibes::newFigure("Ouessant");
    vibes::drawLine(x_position, y_position);
    vibes::axisAuto("Ouessant");
}
