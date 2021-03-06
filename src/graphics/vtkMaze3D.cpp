#include "vtkMaze3D.h"

#include <string>
#include <fstream>
#include <ostream>
#include <iostream>

#include <ibex_IntervalVector.h>

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

#include "smartSubPaving.h"
#include "previmer3d.h"

#include "../serialization/ibex_serialization.h"

using namespace invariant;
using namespace std;
using namespace ibex;

VtkMaze3D::VtkMaze3D(const std::string &file_name, bool memory_optimization){
    m_file_name = file_name;
    m_memory_optimization = memory_optimization;
}

void VtkMaze3D::show_room_info(invariant::Maze<ibex::IntervalVector> *maze, ibex::IntervalVector position){
    std::vector<invariant::Pave<ibex::IntervalVector>*> pave_list;
    maze->get_subpaving()->get_room_info(maze, position, pave_list);
    vector<string> name_files;

    for(invariant::Pave<ibex::IntervalVector>* p:pave_list){
        stringstream file_name;
        file_name << p->get_position();
        name_files.push_back(file_name.str());
    }

    for(size_t i=0; i<pave_list.size(); i++){
        invariant::Pave<ibex::IntervalVector> *p = pave_list[i];
        invariant::Room<ibex::IntervalVector> *r = p->get_rooms()[maze];

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
                invariant::FaceIBEX *f = p->get_faces()[face][sens];
                invariant::DoorIBEX *d = f->get_doors()[maze];

                IntervalVector input(d->get_input());
                IntervalVector output(d->get_output());

                //                input[face] += /*1.0*Interval((sens==1)?(offset[face]):(-offset[face])) +*/ Interval(/*-offset[face]/4.0*/0.0, offset[face]/4.0);
                input[face] += (sens==1)?(ibex::Interval(0, offset[face]/4.0)):(ibex::Interval(-offset[face]/4.0, 0));
                //                output[face] += /*2.0*Interval((sens==1)?(offset[face]):(-offset[face])) +*/ Interval(/*-offset[face]/4.0*/0.0, offset[face]/4.0);
                output[face] += (sens==1)?(ibex::Interval(0, offset[face]/4.0)):(ibex::Interval(-offset[face]/4.0, 0));

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

void VtkMaze3D::serialize_maze(const string &file_name, invariant::MazeIBEX* maze){
    std::ofstream binFile(file_name.c_str(), std::ofstream::out);
    size_t nb_pave = 0;
    binFile.write((const char*)&nb_pave, sizeof(size_t));

    for(size_t pave_id=0; pave_id<maze->get_subpaving()->get_paves().size(); pave_id++){
        PaveIBEX *p = maze->get_subpaving()->get_paves()[pave_id];
        vector<pair<IntervalVector, IntervalVector>> list_doors;

        for(size_t direction=0; direction<3; direction++){
            for(size_t sens = 0; sens <2; sens++){
                invariant::FaceIBEX *f = p->get_faces()[direction][sens];
                invariant::DoorIBEX *d = f->get_doors()[maze];
                if(!d->is_empty()){
                    IntervalVector iv = d->get_input() | d->get_output();
                    IntervalVector orientation = f->get_orientation();
                    list_doors.push_back(std::make_pair(iv, orientation));
                }
            }
        }
        if(!list_doors.empty()){
            size_t size = list_doors.size();
            binFile.write((const char*)&size, sizeof(size_t));
            for(size_t i=0; i<size; i++){
                serializeIntervalVector(binFile, list_doors[i].first);
                serializeIntervalVector(binFile, list_doors[i].second);
            }
            nb_pave++;
        }
    }
    binFile.seekp(0);
    binFile.write((const char*)&nb_pave, sizeof(size_t));

    binFile.close();
}

void VtkMaze3D::show_graph(invariant::SmartSubPavingIBEX* subpaving){
    cout << "vtk paving" << endl;

    vtkSmartPointer<vtkAppendPolyData> polyData_paves = vtkSmartPointer<vtkAppendPolyData>::New();
    int nb_paves = subpaving->get_paves().size();

#pragma omp parallel for
    for(int pave_id=0; pave_id<nb_paves; pave_id++){
        PaveIBEX *p = subpaving->get_paves()[pave_id];
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
    //    for(Pave *p:subpaving->get_paves_not_bisectable()){
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

void VtkMaze3D::show_maze(const string &file_name){
    std::ifstream binFile(file_name.c_str(), std::ifstream::in);

    size_t nb_pave = 0;
    binFile.read((char*)&nb_pave, sizeof(size_t));
    cout << "nb of pave to read = " << nb_pave << endl;

    vtkSmartPointer<vtkAppendPolyData> polyData_polygon = vtkSmartPointer<vtkAppendPolyData>::New();
    size_t step=0;
    for(size_t n = 0; n<nb_pave; n++){
        int nb_points = 0;
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

        size_t nb_doors;
        binFile.read((char*)&nb_doors, sizeof(size_t));
        for(size_t d=0; d<nb_doors; d++){

            IntervalVector iv = deserializeIntervalVector(binFile);
            IntervalVector orientation = deserializeIntervalVector(binFile);
            int val_max[3] = {2, 2, 2};
            for(int i=0; i<3; i++){
                if(orientation[i] != ibex::Interval(0, 1)){
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

        vtkIdType pointIds[nb_points];
        for(int i=0; i<nb_points; i++){
            pointIds[i] = i;
        }
        vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();

        for(size_t i=0; i<nb_doors; i++){
            vtkIdType face[4];
            for(size_t j=0; j<4; j++)
                face[j] = 4*i+j;
            faces->InsertNextCell(4, face);
        }

        vtkSmartPointer<vtkUnstructuredGrid> ugrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
        ugrid->SetPoints(points);
        ugrid->InsertNextCell(VTK_POLYHEDRON, nb_points, pointIds/*, nb_doors, faces->GetPointer()*/); // ToDo : bug fix with VTK9.0 ?

        // ********** Surface **************
        // Create the convex hull of the pointcloud (delaunay + outer surface)
        vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
        delaunay->SetInputData(ugrid);
        delaunay->SetTolerance(0.0);
        delaunay->Update();

        vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
        surfaceFilter->Update();

        polyData_polygon->AddInputData(surfaceFilter->GetOutput());
        if(step%1000==0)
            cout << "step = " << step << " / " << nb_pave << " " << round((step/nb_pave)*100) << "%"<< endl;
        step++;
    }


    polyData_polygon->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = m_file_name + "_polygon" + ".vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(polyData_polygon->GetOutput());
    outputWriter->Write();
}

void VtkMaze3D::show_maze(invariant::MazeIBEX *maze, std::string comment){
    cout << "vtk maze" << endl;
    vtkSmartPointer<vtkAppendPolyData> polyData_polygon = vtkSmartPointer<vtkAppendPolyData>::New();
    //    vtkSmartPointer<vtkPoints> vec_field_points = vtkSmartPointer< vtkPoints >::New();
    //    vtkSmartPointer<vtkFloatArray> field = vtkSmartPointer<vtkFloatArray>::New();

    //    vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
    int dim_paves_list = maze->get_subpaving()->get_paves().size();
    int step = 0;

#pragma omp parallel for schedule(dynamic)
    for(int pave_id=0; pave_id<dim_paves_list; pave_id++){
        invariant::PaveIBEX *p = maze->get_subpaving()->get_paves()[pave_id];
        invariant::RoomIBEX *r = p->get_rooms()[maze];
#pragma omp atomic
        step ++;

        if(r->is_full()){
            IntervalVector position(p->get_position());
            vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
            cubedata->SetBounds(position[0].lb(), position[0].ub(),
                    position[1].lb(), position[1].ub(),
                    position[2].lb(), position[2].ub());
            cubedata->Update();
#pragma omp critical(add_polygon)
            {
                polyData_polygon->AddInputData(cubedata->GetOutput());
                if(step%1000==0)
                    cout << "step = " << step << " /" << dim_paves_list << endl;
            }
        }
        else if(!r->is_empty()){
            int nb_faces = 0;
            int nb_points = 0;
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

            for(size_t direction=0; direction<3; direction++){
                for(size_t sens = 0; sens <2; sens++){
                    invariant::FaceIBEX *f = p->get_faces()[direction][sens];
                    invariant::DoorIBEX *d = f->get_doors()[maze];
                    if(!d->is_empty()){
                        nb_faces++;

                        IntervalVector iv = d->get_input() | d->get_output();
                        IntervalVector orientation = f->get_orientation();
                        int val_max[3] = {2, 2, 2};
                        for(int i=0; i<3; i++){
                            if(orientation[i] != ibex::Interval(0, 1)){
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
                //                vtkIdType pointIds[nb_points];
                //                for(int i=0; i<nb_points; i++){
                //                    pointIds[i] = i;
                //                }
                //                vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();

                //                for(int i=0; i<nb_faces; i++){
                //                    vtkIdType face[4];
                //                    for(int j=0; j<4; j++)
                //                        face[j] = 4*i+j;
                //                    faces->InsertNextCell(4, face);
                //                }

                //                vtkSmartPointer<vtkUnstructuredGrid> ugrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
                //                ugrid->SetPoints(points);
                //                ugrid->InsertNextCell(VTK_POLYHEDRON, nb_points, pointIds, nb_faces, faces->GetPointer());

                if(points->GetNumberOfPoints()>3){
                    vtkSmartPointer< vtkPolyData> polydata_points = vtkSmartPointer<vtkPolyData>::New();
                    polydata_points->SetPoints(points);

                    // ********** Surface **************
                    // Create the convex hull of the pointcloud (delaunay + outer surface)
                    vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
                    delaunay->SetInputData(polydata_points);
                    delaunay->SetTolerance(0.0);
                    delaunay->Update();

                    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
                    surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
                    surfaceFilter->Update();

#pragma omp critical(add_polygon)
                    {
                        polyData_polygon->AddInputData(surfaceFilter->GetOutput());
                        if(step%1000==0)
                            cout << "step = " << step << " /" << dim_paves_list << endl;
                    }
                }
            }
        }
        if(m_memory_optimization){
            maze->get_subpaving()->delete_pave(pave_id);
        }
    }

    //    vtkSmartPointer<vtkAppendPolyData> polyData_field = vtkSmartPointer<vtkAppendPolyData>::New();
    //    vtkSmartPointer< vtkPolyData> dataObject = vtkSmartPointer<vtkPolyData>::New();
    //    dataObject->SetPoints(vec_field_points);
    //    dataObject->GetPointData()->SetVectors(field);
    //    polyData_field->AddInputData(dataObject);
    //    polyData_field->Update();
    //    vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    //    vertexFilter->AddInputData(polyData_field->GetOutput()); // Transform points (array) in vertex objects
    //    vertexFilter->Update();

    polyData_polygon->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    std::stringstream file_name;
    file_name << m_file_name << "_polygon" << comment << "_" << m_number_export << ".vtp";
    m_number_export++;
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(polyData_polygon->GetOutput());
    outputWriter->Write();

    //    polyData_field->Update();
    //    file = m_file_name + "_vecField" + comment + ".vtp";
    //    outputWriter->SetFileName(file.c_str());
    //    outputWriter->SetInputData(vertexFilter->GetOutput());
    //    outputWriter->Write();
}

void monteCarlos(invariant::PreviMer3D &pm3d, double t0, double x0, double y0){
    // MonteCarlos integration
    ibex::IntervalVector search_space(pm3d.get_search_space());

    double lambda = 1.5;
    double a1 = (1.0-1.0/(2.0*lambda));
    double a2 = 1.0/(2.0*lambda);
    double dt = 60.0;

    ibex::IntervalVector x(3);
    x[0] = ibex::Interval(t0*pm3d.get_grid_conversion(0));
    x[1] = ibex::Interval(x0*pm3d.get_grid_conversion(1));
    x[2] = ibex::Interval(y0*pm3d.get_grid_conversion(2));
    cout << "Integration MonteCarlos : " << x << endl;

    vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for(int t=0; t<search_space[0].ub()-15*60; t+=dt){
        ibex::IntervalVector f1 = pm3d.eval_vector(x);
        //        x[0] += dt;
        //        x[1] += dt*f1[0][1];
        //        x[2] += dt*f1[0][2];

        IntervalVector x2(x);
        x2[0] += lambda*dt;
        x2[1] += lambda*dt*(f1[1].mid());
        x2[2] += lambda*dt*(f1[2].mid());
        ibex::IntervalVector f2 = pm3d.eval_vector(x2);
        x[0] += dt;
        x[1] += dt*(a1*(f1[1]).mid()+a2*(f2[1]).mid());
        x[2] += dt*(a1*f1[2].mid()+a2*f2[2].mid());

        bool Break = false;
        if(f1[0].is_empty()){
            cout << "ERROR Trajectory f1 " << t << endl;
            Break = true;
        }
        if(f2[0].is_empty()){
            cout << "ERROR Trajectory f2 " << t << endl;
            Break = true;
        }
        if(Break)
            break;
        points->InsertNextPoint(x[0].lb(), x[1].lb(), x[2].lb());
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

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    stringstream file_name;
    file_name << "monte_carlos_" << x0 << "_" << y0 << ".vtp";
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(linesPolyData);
    outputWriter->Write();
}

void draw_map(LambertGrid &g){
    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();

    vtkSmartPointer<vtkPolyData> gridPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points_grid = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
    for(size_t i=0; i<g.get_X().size(); i++){
        for(size_t j=0; j<g.get_X()[0].size(); j++){
            if(g.get_H0()[i][j]==g.get_H0_Fill_Value()){
                vtkIdType id= points_grid->InsertNextPoint(g.get_X()[i][j], g.get_Y()[i][j], g.get_time_min());
                cellArray->InsertNextCell(1, &id);
            }
        }
    }
    gridPolyData->SetPoints(points_grid);
    gridPolyData->SetVerts(cellArray);

    stringstream file_name_ouessant;
    file_name_ouessant << "map.vtp";
    outputWriter->SetFileName(file_name_ouessant.str().c_str());
    outputWriter->SetInputData(gridPolyData);
    outputWriter->Write();
}
