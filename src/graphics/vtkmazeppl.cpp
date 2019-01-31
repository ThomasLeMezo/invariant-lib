#include "vtkmazeppl.h"

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkAppendPolyData.h>
#include <vtkDelaunay3D.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkCubeSource.h>
#include <vtkHull.h>
#include <vtkFloatArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPointData.h>
#include <vtkLine.h>

#include <ostream>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

using namespace invariant;
namespace PPL = Parma_Polyhedra_Library;
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace std;

VtkMazePPL::VtkMazePPL(const string &file_name)
{
    m_file_name = file_name;
}

void VtkMazePPL::show_vector_field(invariant::MazePPL *maze, string comment){
    cout << " ==> vtk vector_field" << endl;

    vtkSmartPointer<vtkAppendPolyData> polyData_vec = vtkSmartPointer<vtkAppendPolyData>::New();
    int nb_paves = maze->get_subpaving()->get_paves().size();

#pragma omp parallel for
    for(int pave_id=0; pave_id<nb_paves; pave_id++){
        PavePPL *p = maze->get_subpaving()->get_paves()[pave_id];
        RoomPPL *r = p->get_rooms()[maze];
        vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
        ibex::IntervalVector vector_field(r->get_vector_fields()[0]);

        // Fit to box size
        double min_diam = p->get_position().min_diam();
        double max_diam = vector_field.max_diam();
        vector_field *= 0.9*(min_diam/max_diam);

        // Move to the center of the box
        vector_field += p->get_position().mid();

        cubedata->SetBounds(vector_field[0].lb(), vector_field[0].ub(),
                vector_field[1].lb(), vector_field[1].ub(),
                vector_field[2].lb(), vector_field[2].ub());
        cubedata->Update();

        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        points->InsertNextPoint(p->get_position()[0].mid(), p->get_position()[1].mid(), p->get_position()[2].mid());
        vtkSmartPointer< vtkPolyData> polydata_points = vtkSmartPointer<vtkPolyData>::New();
        polydata_points->SetPoints(points);

#pragma omp critical
        {
            polyData_vec->AddInputData(cubedata->GetOutput());
            polyData_vec->AddInputData(polydata_points);
        }
    }
    polyData_vec->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    std::stringstream file_name;
    file_name << m_file_name << "_vector_field" << comment << "_" << m_number_export_vector_field << ".vtp";
    m_number_export_paves++;
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(polyData_vec->GetOutput());
    outputWriter->Write();
}

void VtkMazePPL::show_subpaving(invariant::MazePPL *maze, string comment){
    cout << " ==> vtk subpaving" << endl;

    vtkSmartPointer<vtkAppendPolyData> polyData_paves = vtkSmartPointer<vtkAppendPolyData>::New();
    int nb_paves = maze->get_subpaving()->get_paves().size();

#pragma omp parallel for
    for(int pave_id=0; pave_id<nb_paves; pave_id++){
        PavePPL *p = maze->get_subpaving()->get_paves()[pave_id];
        vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
        ibex::IntervalVector position(p->get_position());
        cubedata->SetBounds(position[0].lb(), position[0].ub(),
                position[1].lb(), position[1].ub(),
                position[2].lb(), position[2].ub());
        cubedata->Update();
#pragma omp critical
        {
            polyData_paves->AddInputData(cubedata->GetOutput());
        }
    }
    polyData_paves->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    std::stringstream file_name;
    file_name << m_file_name << "_paves" << comment << "_" << m_number_export_paves << ".vtp";
    m_number_export_paves++;
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(polyData_paves->GetOutput());
    outputWriter->Write();
}

void VtkMazePPL::show_maze(invariant::MazePPL *maze, string comment, bool complementary)
{
    cout << " ==> vtk maze" << endl;
    vtkSmartPointer<vtkAppendPolyData> polyData_polygon = vtkSmartPointer<vtkAppendPolyData>::New();

    int dim_paves_list = maze->get_subpaving()->get_paves().size();
    int step = 0;

    int dim = maze->get_subpaving()->dim();
    int max_generator=0;
    int min_generator=1e5;
    int nb_pave_not_empty=0;

#pragma omp parallel
    {
        PPL::Thread_Init *thread_init = initialize_thread<ppl::C_Polyhedron>();
#pragma omp for schedule(dynamic)
        for(int pave_id=0; pave_id<dim_paves_list; pave_id++){
            PavePPL *p = maze->get_subpaving()->get_paves()[pave_id];
            RoomPPL *r = p->get_rooms()[maze];
            //#pragma omp atomic
            step ++;

            if(!r->is_empty()){
                nb_pave_not_empty++;
                vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

                ppl::C_Polyhedron ph_union(dim, ppl::EMPTY);

                for(FacePPL *f:p->get_faces_vector()){
                    DoorPPL *d = f->get_doors()[maze];
                    ph_union |= d->get_hull();
                }

                if(r->is_initial_door_input())
                    ph_union |= r->get_initial_door_input();
                if(r->is_initial_door_output())
                    ph_union |= r->get_initial_door_output();

                //            ph_union &= p->get_position_typed();
                ph_union.minimized_constraints();

                if(complementary){
                  ppl::C_Polyhedron position(p->get_position_typed());
                  position.difference_assign(ph_union);
                  ph_union = position;
                }

                if(ph_union.space_dimension()==3){
                    int nb_generator = 0;
                    for(auto &g:ph_union.generators()){
                        nb_generator++;
                        if(g.is_point() && g.space_dimension()==3){
                            std::array<double, 3> coord;
                            for(size_t i=0; i<3; i++){
                                ppl::Variable x(i);
                                mpq_class number(g.coefficient(x), g.divisor());
                                coord[i] = number.get_d();
                            }
                            points->InsertNextPoint(coord[0], coord[1], coord[2]);
                        }
                    }
                    if(max_generator<nb_generator)
                        max_generator = nb_generator;
                    if(min_generator>nb_generator)
                        min_generator = nb_generator;
                    //                cout << nb_points << endl;

                    if(points->GetNumberOfPoints()>3){
                        vtkSmartPointer< vtkPolyData> polydata_points = vtkSmartPointer<vtkPolyData>::New();
                        polydata_points->SetPoints(points);

                        // ********** Surface **************
                        // Create the convex hull of the pointcloud (delaunay + outer surface)
                        //                        vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
                        //                        delaunay->SetInputData(polydata_points);
                        //                        delaunay->Update();

                        vtkSmartPointer<vtkHull> hullFilter = vtkSmartPointer<vtkHull>::New();
                        hullFilter->SetInputData(polydata_points);
                        hullFilter->AddCubeFacePlanes();
                        hullFilter->Update();

                        vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
//                        surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
                        surfaceFilter->SetInputConnection(hullFilter->GetOutputPort());
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
        }
        delete_thread_init<ppl::C_Polyhedron>(thread_init);
    }

    polyData_polygon->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    std::stringstream file_name;
    file_name << m_file_name << "_polygon" << comment << "_" << m_number_export << ".vtp";
    m_number_export++;
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(polyData_polygon->GetOutput());
    outputWriter->Write();

    cout << " ==> generators (max/min) = " << max_generator << "/" << min_generator << endl;
    cout << " ==> nb pave drawn = " << nb_pave_not_empty << endl;
}

void VtkMazePPL::show_room_info(invariant::MazePPL *maze, ibex::IntervalVector position){
    std::vector<invariant::Pave<Parma_Polyhedra_Library::C_Polyhedron>*> pave_list;
    maze->get_subpaving()->get_room_info(maze, position, pave_list);
    vector<string> name_files;

    for(invariant::Pave<Parma_Polyhedra_Library::C_Polyhedron>* p:pave_list){
        stringstream file_name;
        file_name << p->get_position();
        name_files.push_back(file_name.str());
    }

    for(size_t i=0; i<pave_list.size(); i++){
        invariant::Pave<Parma_Polyhedra_Library::C_Polyhedron> *p = pave_list[i];
        invariant::Room<Parma_Polyhedra_Library::C_Polyhedron> *r = p->get_rooms()[maze];

        /// ************ Draw Paves ************
        vtkSmartPointer<vtkCubeSource> cubedata = vtkSmartPointer<vtkCubeSource>::New();
        ibex::IntervalVector position(p->get_position());
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

        ibex::IntervalVector vec_field = r->get_vector_fields()[0];
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
        ibex::IntervalVector p_position(p->get_position());
        double p_diam[3] = {p_position[0].diam(), p_position[1].diam(), p_position[2].diam()};
        double offset[3] = {p_diam[0]*0.05, p_diam[1]*0.05, p_diam[2]*0.05};

        vtkSmartPointer<vtkAppendPolyData> polyData_doors_input = vtkSmartPointer<vtkAppendPolyData>::New();
        vtkSmartPointer<vtkAppendPolyData> polyData_doors_output = vtkSmartPointer<vtkAppendPolyData>::New();
        for(size_t face=0; face<3; face++){
            for(size_t sens=0; sens<2; sens++){
                invariant::FacePPL *f = p->get_faces()[face][sens];
                invariant::DoorPPL *d = f->get_doors()[maze];

                ibex::IntervalVector input(polyhedron_2_iv(d->get_input()));
                ibex::IntervalVector output(polyhedron_2_iv(d->get_output()));

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

void VtkMazePPL::simu_trajectory(ibex::Function *f, const vector<double> &position_init, const double &t_max, const double &dt, const vector<double> &visu_scale){
    vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    ibex::IntervalVector x(3);
    x[0] = ibex::Interval(position_init[0]);
    x[1] = ibex::Interval(position_init[1]);
    x[2] = ibex::Interval(position_init[2]);

    for(double t=0; t<t_max; t+=dt){
        ibex::IntervalVector dx = f->eval_vector(x);
        x[0] = ibex::Interval(x[0].mid()+(dx[0]*dt).mid());
        x[1] = ibex::Interval(x[1].mid()+(dx[1]*dt).mid());
        x[2] = ibex::Interval(x[2].mid()+(dx[2]*dt).mid());
        points->InsertNextPoint(x[0].mid()*visu_scale[0], x[1].mid()*visu_scale[1], x[2].mid()*visu_scale[2]);
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
//    file_name << "trajectory_" << position_init[0] << "_" << position_init[1] << "_" << position_init[2] << ".vtp";
    file_name << "trajectory_" << m_traj_num++ << ".vtp";
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(linesPolyData);
    outputWriter->Write();
}
