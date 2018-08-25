#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>

#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>

#include <vtkPoints.h>
#include <vtkCubeSource.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkDelaunay3D.h>
#include <vtkDataSetSurfaceFilter.h>

#include <vtkUnstructuredGrid.h>

#include <vtkFloatArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPointData.h>

#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <omp.h>

using namespace std;
using namespace invariant;


int main(int argc, char *argv[]){
    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");

    double time_start = omp_get_wtime();

    const double dt = 1.0;
    const double t_init = 3708553500;
    const double x1 = 106354.57; const double y1 = 6846898.61;
    const double x2 = 107187.44; const double y2 = 6845525.38;
    const double duration = 3600*20.0;

    vector<double> time;
    for(double t=0; t<duration; t+=dt)
        time.push_back(t);
    cout << "Time steps = " << time.size() << endl;

    size_t nb_steps = 20;
    vector<vector<double>> theta_memory;
    for(double k=0.0; k<=1.0; k+=1.0/nb_steps){
        cout << "Compute k= " << k << endl;
        double u, v;
        vector<double> theta;
        for(double &t:time){
            if(!g.eval(x1+(x2-x1)*k, y1+(y2-y1)*k, t+t_init, u, v)){
                cout << "Break" << endl;
                break;
            }
            double tmp = atan2(v, u);
//            if(tmp<0)
//                tmp +=2*M_PI;
            theta.push_back(tmp);
        }
        theta_memory.push_back(theta);
    }

    vibes::beginDrawing();
    vibes::newFigure("Fromveur");
    for(vector<double> &theta:theta_memory)
        vibes::drawLine(time, theta);
    vibes::axisAuto();

    cout << "TIME = " << omp_get_wtime() - time_start << endl;
}
