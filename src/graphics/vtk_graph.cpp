#include "vtk_graph.h"
#include <string>

#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkAppendPolyData.h>

#include <vtkPoints.h>
#include <vtkCubeSource.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkDelaunay3D.h>
#include <vtkDataSetSurfaceFilter.h>

using namespace invariant;
using namespace std;
using namespace ibex;

Vtk_Graph::Vtk_Graph(const std::string &file_name, Graph *g){
    m_graph = g;
    m_file_name = file_name;
}

void Vtk_Graph::show_graph(){
    cout << "vtk graph" << endl;

    vtkSmartPointer<vtkAppendPolyData> polyData_paves = vtkSmartPointer<vtkAppendPolyData>::New();

    for(Pave *p:m_graph->get_paves()){
        vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();

        IntervalVector position(p->get_position());
        cubedata->SetBounds(position[0].lb(), position[0].ub(),
                position[1].lb(), position[1].ub(),
                position[2].lb(), position[2].ub());
        cubedata->Update();
        polyData_paves->AddInputData(cubedata->GetOutput());
    }
    polyData_paves->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = m_file_name + "_paves.vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(polyData_paves->GetOutput());
    outputWriter->Write();
}

void Vtk_Graph::show_maze(invariant::Maze *maze){
    cout << "vtk maze" << endl;
    vtkSmartPointer<vtkAppendPolyData> polyData_polygon = vtkSmartPointer<vtkAppendPolyData>::New();

    for(Pave *p:m_graph->get_paves()){
        Room *r = p->get_rooms()[maze];

        if(r->is_full()){
            vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();

            IntervalVector position(p->get_position());
            cubedata->SetBounds(position[0].lb(), position[0].ub(),
                    position[1].lb(), position[1].ub(),
                    position[2].lb(), position[2].ub());
            cubedata->Update();
            polyData_polygon->AddInputData(cubedata->GetOutput());
        }
        else if(!r->is_empty()){
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

            for(size_t direction=0; direction<3; direction++){
                for(size_t sens = 0; sens <2; sens++){
                    Face *f = p->get_faces()[direction][sens];
                    Door *d = f->get_doors()[maze];
                    if(!d->is_empty()){

                        IntervalVector iv = d->get_input() | d->get_output();
                        //                tab_min_max int[3][2] = {{0,2}, {0, 2}, {0, 2}};
                        if(!iv.is_empty()){
                            for(int x=0; x<2; x++){
                                if(iv[0].is_degenerated())
                                    x=1;
                                for(int y=0; y<2; y++){
                                    if(iv[1].is_degenerated())
                                        y=1;
                                    for(int z=0; z<2; z++){
                                        if(iv[2].is_degenerated())
                                            z=1;
                                        points->InsertNextPoint((x==0)?iv[0].lb():iv[0].ub(),
                                                (y==0)?iv[1].lb():iv[1].ub(),
                                            (z==0)?iv[2].lb():iv[2].ub());
                                    }
                                }
                            }
                        }
                    }

                }
            }

            if(points->GetNumberOfPoints() != 0){

                vtkSmartPointer< vtkPolyData> pointsCollection = vtkSmartPointer<vtkPolyData>::New();
                pointsCollection->SetPoints(points);

                // ********** Surface **************
                // Create the convex hull of the pointcloud (delaunay + outer surface)
                vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
                delaunay->SetInputData(pointsCollection);
                delaunay->Update();

                vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
                surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
                surfaceFilter->Update();

                // ********** Append results **************
                polyData_polygon->AddInputData(surfaceFilter->GetOutput());
            }
        }
    }

    polyData_polygon->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = m_file_name + "_polygon.vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(polyData_polygon->GetOutput());
    outputWriter->Write();
}
