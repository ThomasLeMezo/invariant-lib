#include "lambertgrid.h"
#include <iostream>
#include <cmath>
#include <vibes/vibes.h>

#include "vtkMaze3D.h"

#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>

#include <vtkPoints.h>
#include <vtkCubeSource.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkDelaunay3D.h>
#include <vtkDataSetSurfaceFilter.h>

#include <vtkUnstructuredGrid.h>
#include <vtkPolyDataCollection.h>

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
//    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files_rade.xml");
//    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");


    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files_irene.xml");

//    omp_set_num_threads(1);
//    draw_map(g);

    double time_start = omp_get_wtime();

    // Compute trajectories
    double x_init, y_init, t_init;
    double dt = 1.0;
//    t_init = 1541394000; //
//    x_init = 149357;
//    y_init = 6828729;
//    double t_end = 3600*6.0;

    t_init = 1294866240; //
    x_init = 95759;
    y_init = 6846047;
    double t_end = 25.0*3600.0;
    cout << "time_min = " << std::setprecision(10) << g.get_time_min() << endl;
    cout << "time_max = " << g.get_time_max() << endl;

    bool plot_unfinished_trajectories = true;

    vector<array<double, 3>> init_conditions;
    for(double x=-1; x<=1; x++){
        for(double y=-1; y<=1; y++){
            for(double t=-1; t<=1; t++){
                init_conditions.push_back(array<double, 3>{x_init+5.0*x, y_init+5*y, t_init+t*60.});
            }
        }
    }

    vtkSmartPointer<vtkAppendPolyData> trajectories = vtkSmartPointer<vtkAppendPolyData>::New();

    const size_t nb_conditions = init_conditions.size();
#pragma omp parallel for
    for(size_t k=0; k<nb_conditions; k++){
            double x_tmp = init_conditions[k][0];
            double y_tmp = init_conditions[k][1];
            double t_tmp = init_conditions[k][2];
            bool valid = true;
#pragma omp critical
            {
            cout << "> Compute Traj " << k << endl;
            }

            vtkSmartPointer<vtkPolyData> trajectory = vtkSmartPointer<vtkPolyData>::New();
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
            for(double t=t_tmp; t<t_tmp+t_end; t+=dt){
                // RK2 scheme
                double u, v;
                g.eval(x_tmp, y_tmp, t, u, v);
                double x_r = x_tmp + (dt/2.0)*u;
                double y_r = y_tmp + (dt/2.0)*v;

                int err = g.eval(x_r, y_r, t+(dt/2.0), u, v);
                if(err !=0){
                    valid = false;
                    cout << "Error " << err << endl;
                }

                x_tmp+=u*dt;
                y_tmp+=v*dt;

                points->InsertNextPoint(x_tmp, y_tmp, t+dt-t_init);
                if(!valid)
                    break;
            }
            trajectory->SetPoints(points);

            vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
            for(unsigned int i = 0; i < points->GetNumberOfPoints()-1; i++){
                vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
                line->GetPointIds()->SetId(0,i);
                line->GetPointIds()->SetId(1,i+1);
                lines->InsertNextCell(line);
            }
            trajectory->SetLines(lines);

//            vtkSmartPointer<vtkIntArray> intValue = vtkSmartPointer<vtkIntArray>::New();
//            intValue->SetNumberOfComponents(1);
//            intValue->SetName("TrajId");
//            intValue->InsertNextValue(k);
//            trajectory->GetFieldData()->AddArray(intValue);

#pragma omp critical
            {
                if(valid || plot_unfinished_trajectories){
                    trajectories->AddInputData(trajectory);
                }
            }
        }

    trajectories->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    stringstream file_name;
    file_name << "trajectories.vtp";
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(trajectories->GetOutput());
    outputWriter->Write();

    cout << "TIME = " << omp_get_wtime() - time_start << endl;
}
