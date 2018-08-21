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

using namespace std;
using namespace invariant;

void draw_Ouessant(LambertGrid &g){
    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();

    vtkSmartPointer<vtkPolyData> gridPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points_grid = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
    for(size_t i=0; i<g.get_X().size(); i++){
        for(size_t j=0; j<g.get_X()[0].size(); j++){
            if(g.get_H0()[i][j]==g.get_H0_Fill_Value()){
                vtkIdType id= points_grid->InsertNextPoint(g.get_X()[i][j], g.get_Y()[i][j], 0.0);
                cellArray->InsertNextCell(1, &id);
            }
        }
    }
    gridPolyData->SetPoints(points_grid);
    gridPolyData->SetVerts(cellArray);

    stringstream file_name_ouessant;
    file_name_ouessant << "ouessant.vtp";
    outputWriter->SetFileName(file_name_ouessant.str().c_str());
    outputWriter->SetInputData(gridPolyData);
    outputWriter->Write();
}


int main(int argc, char *argv[]){
    LambertGrid g("/home/lemezoth/Documents/ensta/flotteur/data_ifremer/files.xml");

    // Draw Ouessant
//    draw_Ouessant();

    // Compute trajectories
    double x_init, y_init, t_init, u, v;

    double dt = 1.0;

    t_init = 3708553500;
    x_init = 97339+250*3;
    y_init = 6848741;

    double t_start = 0;
    double t_end = 3600*20.0;
    size_t k=0;

    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

#pragma omp parallel for
    for(int i=0; i<5; i++){
        double x = x_init+50.0*i;
        for(double y=y_init; y<y_init+250; y+=50.0){
            double x_tmp = x;
            double y_tmp = y;
            bool valid = true;
#pragma omp critical
            {
            cout << "> Compute Traj " << k++ << endl;
            }

            vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
            for(double t=t_init+t_start; t<t_init+t_end; t+=dt){
                // RK2 scheme
                g.eval(x_tmp, y_tmp, t, u, v);
                double x_r = x_tmp + dt/2.0*u;
                double y_r = y_tmp + dt/2.0*v;

                if(!g.eval(x_r, y_r, t+dt/2.0, u, v))
                    valid = false;

                x_tmp+=u*dt;
                y_tmp+=v*dt;

                points->InsertNextPoint(x_tmp, y_tmp, t);
                if(!valid)
                    t=t_init+t_end;
            }
            linesPolyData->SetPoints(points);

            vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
            for(unsigned int i = 0; i < points->GetNumberOfPoints()-1; i++){
                vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
                line->GetPointIds()->SetId(0,i);
                line->GetPointIds()->SetId(1,i+1);
                lines->InsertNextCell(line);
            }
            linesPolyData->SetLines(lines);

#pragma omp critical
            {
                if(valid)
                    appendFilter->AddInputData(linesPolyData);
            }
        }
    }
    appendFilter->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    stringstream file_name;
    file_name << "monte_carlos.vtp";
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(appendFilter->GetOutput());
    outputWriter->Write();
}
