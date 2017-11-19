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
#include <ostream>

#include <vtkFloatArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPointData.h>

using namespace invariant;
using namespace std;
using namespace ibex;

Vtk_Graph::Vtk_Graph(const std::string &file_name, SmartSubPaving *g, bool memory_optimization){
    m_subpaving = g;
    m_file_name = file_name;
    m_memory_optimization = memory_optimization;
}

void Vtk_Graph::show_room_info(invariant::Maze *maze, ibex::IntervalVector position){
    std::vector<invariant::Pave*> pave_list;
    m_subpaving->get_room_info(maze, position, pave_list);
    vector<string> name_files;

    for(invariant::Pave* p:pave_list){
        stringstream file_name;
        file_name << p->get_position();
        name_files.push_back(file_name.str());
    }

    for(size_t i=0; i<pave_list.size(); i++){
        Pave *p = pave_list[i];
        Room *r = p->get_rooms()[maze];

        /// ************ Draw Paves ************
        vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
        IntervalVector position(p->get_position());
        cubedata->SetBounds(position[0].lb(), position[0].ub(),
                position[1].lb(), position[1].ub(),
                position[2].lb(), position[2].ub());
        cubedata->Update();
        vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
        outputWriter->SetFileName((name_files[i] + "_paves.vtp").c_str());
        outputWriter->SetInputData(cubedata->GetOutput());
        outputWriter->Write();

        /// ************ Draw field ************
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer< vtkPoints >::New();
        vtkSmartPointer<vtkFloatArray> field = vtkSmartPointer<vtkFloatArray>::New();
        vtkSmartPointer< vtkPolyData> dataObject = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();

        field->SetNumberOfComponents(3);
        field->SetName("Glyph");

        IntervalVector vec_field = r->get_vector_fields()[0];
        double vec_field_point[3][2] = {{vec_field[0].lb(), vec_field[0].ub()},
                                        {vec_field[1].lb(), vec_field[1].ub()},
                                        {vec_field[2].lb(), vec_field[2].ub()}};
        field->InsertNextTuple3(vec_field[0].mid(),
                vec_field[1].mid(),
                vec_field[2].mid());
        for(int x=0; x<2; x++){
            for(int y=0; y<2; y++){
                for(int z=0; z<2; z++){
                    points->InsertNextPoint(position[0].mid(), position[1].mid(), position[2].mid());
                    field->InsertNextTuple3(vec_field_point[0][x],
                            vec_field_point[1][y],
                            vec_field_point[2][z]);
                }
            }
        }

        dataObject->SetPoints(points);
        dataObject->GetPointData()->SetVectors(field);

        vertexFilter->SetInputData(dataObject); // Transform points (array) in vertex objects
        vertexFilter->Update();

        outputWriter->SetFileName((name_files[i] + "_vector_field.vtp").c_str());
        outputWriter->SetInputData(vertexFilter->GetOutput());
        outputWriter->Write();

        /// ************ Draw Faces ************
        IntervalVector p_position(p->get_position());
        double p_diam[3] = {p_position[0].diam(), p_position[1].diam(), p_position[2].diam()};
        double offset[3] = {p_diam[0]*0.05, p_diam[1]*0.05, p_diam[2]*0.05};

        vtkSmartPointer<vtkAppendPolyData> polyData_doors_input = vtkSmartPointer<vtkAppendPolyData>::New();
        vtkSmartPointer<vtkAppendPolyData> polyData_doors_output = vtkSmartPointer<vtkAppendPolyData>::New();
        for(size_t face=0; face<3; face++){
            for(size_t sens=0; sens<2; sens++){
                Face *f = p->get_faces()[face][sens];
                Door *d = f->get_doors()[maze];

                IntervalVector input(d->get_input());
                IntervalVector output(d->get_output());

                //                input[face] += /*1.0*Interval((sens==1)?(offset[face]):(-offset[face])) +*/ Interval(/*-offset[face]/4.0*/0.0, offset[face]/4.0);
                input[face] += (sens==1)?(Interval(0, offset[face]/4.0)):(Interval(-offset[face]/4.0, 0));
                //                output[face] += /*2.0*Interval((sens==1)?(offset[face]):(-offset[face])) +*/ Interval(/*-offset[face]/4.0*/0.0, offset[face]/4.0);
                output[face] += (sens==1)?(Interval(0, offset[face]/4.0)):(Interval(-offset[face]/4.0, 0));

                if(!input.is_empty()){
                    vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
                    cubedata->SetBounds(input[0].lb(), input[0].ub(),
                            input[1].lb(), input[1].ub(),
                            input[2].lb(), input[2].ub());
                    cubedata->Update();
                    polyData_doors_input->AddInputData(cubedata->GetOutput());
                }

                if(!output.is_empty()){
                    vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
                    cubedata->SetBounds(output[0].lb(), output[0].ub(),
                            output[1].lb(), output[1].ub(),
                            output[2].lb(), output[2].ub());
                    cubedata->Update();
                    polyData_doors_output->AddInputData(cubedata->GetOutput());
                }
            }
        }

        polyData_doors_input->Update();
        outputWriter->SetFileName((name_files[i] + "_doors_input.vtp").c_str());
        outputWriter->SetInputData(polyData_doors_input->GetOutput());
        outputWriter->Write();

        polyData_doors_output->Update();
        outputWriter->SetFileName((name_files[i] + "_doors_output.vtp").c_str());
        outputWriter->SetInputData(polyData_doors_output->GetOutput());
        outputWriter->Write();
    }
}

void Vtk_Graph::show_graph(){
    cout << "vtk paving" << endl;

    vtkSmartPointer<vtkAppendPolyData> polyData_paves = vtkSmartPointer<vtkAppendPolyData>::New();
    int nb_paves = m_subpaving->get_paves().size();

#pragma omp parallel for
    for(int pave_id=0; pave_id<nb_paves; pave_id++){
        Pave *p = m_subpaving->get_paves()[pave_id];
        vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
        IntervalVector position(p->get_position());
        cubedata->SetBounds(position[0].lb(), position[0].ub(),
                position[1].lb(), position[1].ub(),
                position[2].lb(), position[2].ub());
        cubedata->Update();
#pragma omp critical
        {
            polyData_paves->AddInputData(cubedata->GetOutput());
        }
    }
    //    for(Pave *p:m_subpaving->get_paves_not_bisectable()){
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
    vtkSmartPointer<vtkAppendPolyData> polyData_field = vtkSmartPointer<vtkAppendPolyData>::New();

    //    vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
    int dim_paves_list = m_subpaving->get_paves().size();
    int step = 0;

#pragma omp parallel for schedule(dynamic)
    for(int pave_id=0; pave_id<dim_paves_list; pave_id++){
        Pave *p = m_subpaving->get_paves()[pave_id];
        Room *r = p->get_rooms()[maze];
#pragma omp atomic
        step ++;

        if(r->is_full()){
            //            IntervalVector position(p->get_position());
            //            cubedata->SetBounds(position[0].lb(), position[0].ub(),
            //                    position[1].lb(), position[1].ub(),
            //                    position[2].lb(), position[2].ub());
            //            cubedata->Update();
            //#pragma omp critical(add_polygon)
            //            {
            //                polyData_polygon->AddInputData(cubedata->GetOutput());
            //                if(step%1000==0)
            //                    cout << "step = " << step << " /" << dim_paves_list << endl;
            //            }
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

                // Vector field work
                vtkSmartPointer<vtkPoints> points = vtkSmartPointer< vtkPoints >::New();
                vtkSmartPointer<vtkFloatArray> field = vtkSmartPointer<vtkFloatArray>::New();
                vtkSmartPointer< vtkPolyData> dataObject = vtkSmartPointer<vtkPolyData>::New();
                vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();

                field->SetNumberOfComponents(3);
                field->SetName("Glyph");

                IntervalVector position(p->get_position());
                IntervalVector vec_field = r->get_vector_fields()[0];
                if(!vec_field.is_empty()){
                    double vec_field_point[3][2] = {{vec_field[0].lb(), vec_field[0].ub()},
                                                    {vec_field[1].lb(), vec_field[1].ub()},
                                                    {vec_field[2].lb(), vec_field[2].ub()}};
                    field->InsertNextTuple3(vec_field[0].mid(),
                            vec_field[1].mid(),
                            vec_field[2].mid());
                    for(int x=0; x<2; x++){
                        for(int y=0; y<2; y++){
                            for(int z=0; z<2; z++){
                                points->InsertNextPoint(position[0].mid(), position[1].mid(), position[2].mid());
                                field->InsertNextTuple3(vec_field_point[0][x],
                                        vec_field_point[1][y],
                                        vec_field_point[2][z]);
                            }
                        }
                    }

                    dataObject->SetPoints(points);
                    dataObject->GetPointData()->SetVectors(field);
                    vertexFilter->AddInputData(dataObject); // Transform points (array) in vertex objects
                    vertexFilter->Update();
                }

#pragma omp critical(add_polygon)
                {
                    polyData_polygon->AddInputData(surfaceFilter->GetOutput());
                    polyData_field->AddInputData(vertexFilter->GetOutput());

                    if(step%1000==0)
                        cout << "step = " << step << " /" << dim_paves_list << endl;
                }
            }
        }
        if(m_memory_optimization){
            m_subpaving->delete_pave(pave_id);
        }
    }

    polyData_polygon->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = m_file_name + "_polygon" + comment + ".vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(polyData_polygon->GetOutput());
    outputWriter->Write();

//    polyData_field->Update();
//    file = m_file_name + "_vecField" + comment + ".vtp";
//    outputWriter->SetFileName(file.c_str());
//    outputWriter->SetInputData(polyData_field->GetOutput());
//    outputWriter->Write();
}
