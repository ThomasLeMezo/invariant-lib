#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>

using namespace std;
using namespace invariant;


int main(int argc, char *argv[]){

    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/data/MARC_L1-MARS2D-FINIS250_20180115T1800Z_MeteoMF.nc");

//    cout << g.get_X()[0][0] << endl;
//    cout << g.get_Y()[0][0] << endl;

//    double d_min = 1e6;
//    double d_max = 0.0;
//    size_t i_max = g.get_X().size();
//    size_t j_max = g.get_X()[0].size();
//    cout << "i_max = " << i_max << endl;
//    cout << "j_max = " << j_max << endl;

//    for(size_t j=0; j<j_max; j++){
//        for(size_t i =0; i<i_max-1; i++){
//            double d = abs(g.get_X()[i][j]-g.get_X()[i+1][j]);
//            if(d>d_max)
//                d_max = d;
//            if(d<d_min)
//                d_min = d;
//        }
//    }
//    cout << "d_max = " << d_max <<endl;
//    cout << " d_min = " << d_min << endl;

//    // Min Max coord
//    double X_min, Y_min, X_max, Y_max;
//    X_min=g.get_X()[0][0];
//    Y_min=g.get_Y()[0][0];
//    X_max = X_min;
//    Y_max = Y_min;
//    for(size_t j=0; j<j_max; j++){
//        for(size_t i =0; i<i_max; i++){
//            double x = g.get_X()[i][j];
//            double y = g.get_Y()[i][j];
//            if(x<X_min)
//                X_min=x;
//            if(y<Y_min)
//                Y_min=y;
//            if(x>X_max)
//                X_max=x;
//            if(y>Y_max)
//                Y_max=y;
//        }
//    }

//    cout << "X (min/max) = " << X_min << " " << X_max << endl;
//    cout << "Y (min/max) = " << Y_min << " " << Y_max << endl;

//    vibes::beginDrawing();
//    vibes::newFigure("LambertGrid");
//    for(size_t i =0; i<i_max; i++)
//        vibes::drawPoints(g.get_X()[i], g.get_Y()[i], "b[b]");
//    vibes::axisAuto("LambertGrid");


}
