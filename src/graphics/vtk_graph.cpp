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
#include <vtkCleanPolyData.h>

#include <vtkSurfaceReconstructionFilter.h>
#include <vtkContourFilter.h>
#include <vtkReverseSense.h>

#include <vtkUnstructuredGrid.h>

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
    //    for(Pave *p:m_graph->get_paves_not_bisectable()){
    //        vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
    //        IntervalVector position(p->get_position());
    //        cubedata->SetBounds(position[0].lb(), position[0].ub(),
    //                position[1].lb(), position[1].ub(),
    //                position[2].lb(), position[2].ub());
    //        cubedata->Update();
    //        polyData_paves->AddInputData(cubedata->GetOutput());
    //    }
    polyData_paves->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = m_file_name + "_paves.vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(polyData_paves->GetOutput());
    outputWriter->Write();
}

void Vtk_Graph::show_maze(invariant::Maze *maze, std::string comment){
    cout << "vtk maze" << endl;
    vtkSmartPointer<vtkAppendPolyData> polyData_polygon = vtkSmartPointer<vtkAppendPolyData>::New();
    vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
    int dim_paves_list = m_graph->get_paves().size();

#pragma omp parallel for
    for(int pave_id=0; pave_id<dim_paves_list; pave_id++){
        Pave *p = m_graph->get_paves()[pave_id];
        Room *r = p->get_rooms()[maze];


        if(r->is_full()){
            IntervalVector position(p->get_position());
            cubedata->SetBounds(position[0].lb(), position[0].ub(),
                    position[1].lb(), position[1].ub(),
                    position[2].lb(), position[2].ub());
            cubedata->Update();
            polyData_polygon->AddInputData(cubedata->GetOutput());
        }
        else if(!r->is_empty()){
            int nb_faces = 0;
            int nb_points = 0;
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

            for(size_t direction=0; direction<3; direction++){
                for(size_t sens = 0; sens <2; sens++){
                    Face *f = p->get_faces()[direction][sens];
                    Door *d = f->get_doors()[maze];
                    if(!d->is_empty()){
                        nb_faces++;

                        IntervalVector iv = d->get_input() | d->get_output();
                        IntervalVector orientation = f->get_orientation();
                        int val_max[3] = {2, 2, 2};
                        for(int i=0; i<3; i++){
                            if(orientation[i] != Interval(0, 1)){
                                val_max[i] = 1;
                                break;
                            }
                        }

                        double val[3][2] = {{iv[0].lb(), iv[0].ub()}, // x
                                            {iv[1].lb(), iv[1].ub()},  // y
                                            {iv[2].lb(), iv[2].ub()}}; // z

                        for(int x=0; x<val_max[0]; x++){
                            for(int y=0; y<val_max[1]; y++){
                                for(int z=0; z<val_max[2]; z++){
                                    points->InsertNextPoint(val[0][x], val[1][y], val[2][z]);
                                    nb_points++;
                                }
                            }
                        }
                    }

                }
            }

            if(nb_faces>0){
                vtkIdType pointIds[nb_points];
                for(int i=0; i<nb_points; i++){
                    pointIds[i] = i;
                }
                vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();

                for(int i=0; i<nb_faces; i++){
                    vtkIdType face[4];
                    for(int j=0; j<4; j++)
                        face[j] = 4*i+j;
                    faces->InsertNextCell(4, face);
                }

                vtkSmartPointer<vtkUnstructuredGrid> ugrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
                ugrid->SetPoints(points);
                ugrid->InsertNextCell(VTK_POLYHEDRON, nb_points, pointIds, nb_faces, faces->GetPointer());

                // ********** Surface **************
                // Create the convex hull of the pointcloud (delaunay + outer surface)
                vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
                delaunay->SetInputData(ugrid);
                delaunay->SetTolerance(0.0);
                delaunay->Update();

                vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
                surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
                surfaceFilter->Update();

                // Sometimes the contouring algorithm can create a volume whose gradient
                // vector and ordering of polygon (using the right hand rule) are
                // inconsistent. vtkReverseSense cures this problem.
                //            vtkSmartPointer<vtkReverseSense> reverse = vtkSmartPointer<vtkReverseSense>::New();
                //            reverse->SetInputConnection(cf->GetOutputPort());
                //            reverse->ReverseCellsOn();
                //            reverse->ReverseNormalsOn();
                //            reverse->Update();

                // ********** Append results **************
                #pragma omp critical
                {
                polyData_polygon->AddInputData(surfaceFilter->GetOutput());
                }
            }
        }
    }

    polyData_polygon->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = m_file_name + "_polygon" + comment + ".vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(polyData_polygon->GetOutput());
    outputWriter->Write();
}
