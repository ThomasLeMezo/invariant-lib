#include "vtkmazeppl.h"

#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkAppendPolyData.h"
#include "vtkDelaunay3D.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkXMLPolyDataWriter.h"

#include <ostream>
#include <iostream>
#include <fstream>
#include <string>

using namespace invariant;

VtkMazePPL::VtkMazePPL(const string &file_name)
{
    m_file_name = file_name;
}

void VtkMazePPL::show_maze(invariant::MazePPL *maze, string comment)
{
    cout << "vtk maze" << endl;
    vtkSmartPointer<vtkAppendPolyData> polyData_polygon = vtkSmartPointer<vtkAppendPolyData>::New();

    int dim_paves_list = maze->get_subpaving()->get_paves().size();
    int step = 0;

    int dim = maze->get_subpaving()->dim();
    int max_generator=0;
    int min_generator=1e5;

//#pragma omp parallel for schedule(dynamic)
    for(int pave_id=0; pave_id<dim_paves_list; pave_id++){
        PavePPL *p = maze->get_subpaving()->get_paves()[pave_id];
        RoomPPL *r = p->get_rooms()[maze];
//#pragma omp atomic
        step ++;

        if(!r->is_empty()){
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

            ppl::C_Polyhedron ph_union(dim, ppl::EMPTY);

            for(FacePPL *f:p->get_faces_vector()){
                DoorPPL *d = f->get_doors()[maze];
                ph_union |= d->get_hull();
            }
            ph_union.minimized_constraints();

            if(ph_union.space_dimension()==3){
                int nb_generator = 0;
                for(auto &g:ph_union.generators()){
                    nb_generator++;
                    if(g.is_point()){
                        std::vector<double> coord;
                        for(size_t i=0; i<3; i++){
                            ppl::Variable x(i);
                            if(g.space_dimension()>i){
                                coord.push_back(g.coefficient(x).get_d()/(g.divisor().get_d()*IBEX_PPL_PRECISION));
                            }
                            else{
                                coord.push_back(0.0); // ?
                            }
                        }
//                        points->InsertNextPoint(coord[0], coord[1], coord[2]);
                    }
                }
                if(max_generator<nb_generator)
                    max_generator = nb_generator;
                if(min_generator>nb_generator)
                    min_generator = nb_generator;
//                cout << nb_points << endl;
            }

            if(points->GetNumberOfPoints()>0){
                vtkSmartPointer< vtkPolyData> polydata_points = vtkSmartPointer<vtkPolyData>::New();
                  polydata_points->SetPoints(points);

                // ********** Surface **************
                // Create the convex hull of the pointcloud (delaunay + outer surface)
                vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
                delaunay->SetInputData(polydata_points);
                delaunay->Update();

                vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
                surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
                surfaceFilter->Update();


//#pragma omp critical(add_polygon)
                {
                    polyData_polygon->AddInputData(surfaceFilter->GetOutput());
                    if(step%1000==0)
                        cout << "step = " << step << " /" << dim_paves_list << endl;
                }
            }
        }
    }

    polyData_polygon->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    std::stringstream file_name;
    file_name << m_file_name << "_polygon" << comment << "_" << m_number_export << ".vtp";
    m_number_export++;
    outputWriter->SetFileName(file_name.str().c_str());
    outputWriter->SetInputData(polyData_polygon->GetOutput());
    outputWriter->Write();

    cout << "generators (max/min) = " << max_generator << "/" << min_generator << endl;

}


