#include <iostream>
#include <errno.h>
#include <ppl.hh>
#include <ibex.h>
#include <iostream>
#include <omp.h>

#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkDelaunay3D.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkCellArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkFloatArray.h>
#include <vtkVertexGlyphFilter.h>

#include "resultstorage.h"

using namespace std;
namespace PPL = Parma_Polyhedra_Library;
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace ibex;

ppl::C_Polyhedron iv_2_polyhedron(const ibex::IntervalVector& iv){
    size_t iv_size = (size_t) iv.size();
    ppl::Rational_Box box(iv_size);
    if(!iv.is_empty()){
        for(size_t i=0; i<iv_size; i++){
            ppl::Variable x(i);
            if(!std::isinf(iv[i].lb())){ // '::' To avoid bug with Travis
                mpq_class lb(iv[i].lb());
                box.add_constraint(x*lb.get_den() >= lb.get_num());
            }
            if(!std::isinf(iv[i].ub())){
                mpq_class ub(iv[i].ub());
                box.add_constraint(x*ub.get_den() <= ub.get_num());
            }
        }
    }
    else{
        box = ppl::Rational_Box(iv.size(), Parma_Polyhedra_Library::Degenerate_Element::EMPTY);
    }
    return ppl::C_Polyhedron(box);
}

void write_VTK(const std::vector<Parma_Polyhedra_Library::C_Polyhedron> &ph_list, string filename){
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer< vtkPoints >::New();
    cout << "Write " << filename << endl;

    for(auto &ph:ph_list){
        ph.minimized_constraints();

        if(ph.space_dimension()<3){
            for(auto &g:ph.generators()){
                if(g.is_point()){
                    std::vector<double> coord;
                    for(int cpt = 0; cpt < 2; cpt++){
                        coord.clear();
                        for(int i=0; i<3; i++){
                            PPL::Variable x(i);
                            if(i<2){
                                cout << g.coefficient(x).get_d()/(g.divisor().get_d()) << ",";
                                coord.push_back(g.coefficient(x).get_d()/(g.divisor().get_d()));
                            }
                            else{
                                cout << cpt << endl;
                                coord.push_back(cpt/100.0);
                            }
                        }
                        points->InsertNextPoint(coord[0], coord[1], coord[2]);
                    }
                }
            }
        }
        else{
            for(auto &g:ph.generators()){
                if(g.is_point() && g.space_dimension()==3){
                    std::array<double, 3> coord;
                    for(size_t i=0; i<3; i++){
                        ppl::Variable x(i);
                        mpq_class number(g.coefficient(x), g.divisor());
                        coord[i] = number.get_d();
                    }
                    points->InsertNextPoint(coord[0], coord[1], coord[2]);
//                    cout << "coord = " << coord[0] << " " << coord[1] << " " << coord[2] << endl;
                }
            }
        }
    }

    vtkSmartPointer< vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    //polydata->SetVerts(vertices);

    // Create the convex hull of the pointcloud
    vtkSmartPointer<vtkDelaunay3D> delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
    delaunay->SetInputData(polydata);
    delaunay->SetTolerance(0.0);
    delaunay->Update();

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
    surfaceFilter->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    string file = filename + ".vtp";
    outputWriter->SetFileName(file.c_str());
    outputWriter->SetInputData(surfaceFilter->GetOutput());
    outputWriter->Write();
}

void write_VTK(PPL::C_Polyhedron &ph, string filename){
    std::vector<PPL::C_Polyhedron> ph_list;
    ph_list.push_back(ph);
    write_VTK(ph_list, filename);
}

int test(){
    PPL::C_Polyhedron ph1(3);
    PPL::C_Polyhedron ph2(3);

#pragma omp parallel
    {
        PPL::Thread_Init *thread_init = nullptr;
        bool thread_init_valid = true;
#pragma omp master
        {
            thread_init_valid = false;
        }
        if(thread_init_valid)
            thread_init = new PPL::Thread_Init();
#pragma omp for
        for(int i=0; i<4; i++){
            PPL::C_Polyhedron ph3(3);
            ph3.intersection_assign(ph2);
        }
        if(thread_init_valid)
            delete(thread_init);
    }
    cout << omp_get_thread_num() << endl;
    return 0;
}

int test2(){
    PPL::C_Polyhedron ph3(3);
    PPL::C_Polyhedron ph4(3);
    ph3.intersection_assign(ph4);
    return 0;
}

void plot_bisection_example(){
    IntervalVector box(3);
    box[0] = ibex::Interval(-1, 1);
    box[1] = ibex::Interval(-1, 1);
    box[2] = ibex::Interval(-1, 1);

    IntervalVector door1(3);
    door1[0] = ibex::Interval(-1, -1+1e-2);
    door1[1] = ibex::Interval(-1, -0.5);
    door1[2] = ibex::Interval(-1, -0.5);

    IntervalVector vf(3);
    vf[0] = ibex::Interval(1.0);
    vf[1] = ibex::Interval(-0.5, 0.5);
    vf[2] = ibex::Interval(-0.5, 0.5);

    IntervalVector bisect(3);
    bisect[0] = ibex::Interval(-1, 1);
    bisect[1] = ibex::Interval(-1, 1);
    bisect[2] = ibex::Interval(0.0, 1e-3);

    C_Polyhedron ph_box = iv_2_polyhedron(box);
    C_Polyhedron ph_door1 = iv_2_polyhedron(door1);
    C_Polyhedron ph_vf = iv_2_polyhedron(vf);
    C_Polyhedron ph_bisect = iv_2_polyhedron(bisect);

    C_Polyhedron ph_projection(ph_door1);
    ph_projection.time_elapse_assign(ph_vf);
    C_Polyhedron ph_polygon(ph_projection);
    ph_polygon.intersection_assign(ph_box);

    C_Polyhedron ph_polygon_bisect(ph_polygon);
    ph_polygon_bisect.intersection_assign(ph_bisect);

    cout << "ph_box " << ph_box << endl;
    cout << "ph_door1 " << ph_door1 << endl;
    cout << "ph_polygon " << ph_polygon << endl;
    cout << "ph_polygon_bisect " << ph_polygon_bisect << endl;

    write_VTK(ph_box, "ph_box");
    write_VTK(ph_door1, "ph_door1");
    write_VTK(ph_polygon, "ph_polygon");
    write_VTK(ph_bisect, "ph_bisect");
    write_VTK(ph_polygon_bisect, "ph_polygon_bisect");

}

void plot_vf(ibex::IntervalVector &position, std::string name_file){
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer< vtkPoints >::New();
    vtkSmartPointer<vtkFloatArray> field = vtkSmartPointer<vtkFloatArray>::New();
    vtkSmartPointer< vtkPolyData> dataObject = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();

    field->SetNumberOfComponents(3);
    field->SetName("Glyph");

    IntervalVector vec_field(3);
    vec_field[0] = ibex::Interval(-2.0);
    vec_field[1] = ibex::Interval(-2, 2);
    vec_field[2] = ibex::Interval(-2, 2);

//    IntervalVector position(3);
//    position[0] = ibex::Interval(0, 2);
//    position[1] = ibex::Interval(0, 1);
//    position[2] = ibex::Interval(0, 1);

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

    vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    outputWriter->SetFileName(name_file.c_str());
    outputWriter->SetInputData(vertexFilter->GetOutput());
    outputWriter->Write();
}

void plot_trajectory(){
    vtkSmartPointer<vtkPolyData> trajectory = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    double x = 2.0;
    double dt=0.01;
    double freq = 10;
    for(double t=0; t<2; t+=dt){
      x-=dt;
      double y=cos(t*freq-M_PI_2)*0.4+1;
      double z=sin(t*freq-M_PI_2)*0.4+1;
      points->InsertNextPoint(x, y, z);
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

  vtkSmartPointer<vtkXMLPolyDataWriter> outputWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  outputWriter->SetFileName("trajectory.vtp");
  outputWriter->SetInputData(trajectory);
  outputWriter->Write();
}

void plot_helicoidal_example(){
    IntervalVector box1(3);
    box1[0] = ibex::Interval(0, 2);
    box1[1] = ibex::Interval(0, 1);
    box1[2] = ibex::Interval(0, 1);

    IntervalVector box2(3);
    box2[0] = ibex::Interval(0, 2);
    box2[1] = ibex::Interval(0, 1);
    box2[2] = ibex::Interval(1, 2);

    IntervalVector box3(3);
    box3[0] = ibex::Interval(0, 2);
    box3[1] = ibex::Interval(1, 2);
    box3[2] = ibex::Interval(0, 1);

    IntervalVector box4(3);
    box4[0] = ibex::Interval(0, 2);
    box4[1] = ibex::Interval(1, 2);
    box4[2] = ibex::Interval(1, 2);

    IntervalVector door1(3);
    door1[0] = ibex::Interval(0, 2);
    door1[1] = ibex::Interval(0, 1);
    door1[2] = ibex::Interval(1, 1+1e-3);
    IntervalVector door2(3);
    door2[0] = ibex::Interval(0, 2);
    door2[1] = ibex::Interval(1, 2);
    door2[2] = ibex::Interval(1, 1+1e-3);
    IntervalVector door3(3);
    door3[0] = ibex::Interval(0, 2);
    door3[1] = ibex::Interval(1, 1+1e-3);
    door3[2] = ibex::Interval(0,1);
    IntervalVector door4(3);
    door4[0] = ibex::Interval(0, 2);
    door4[1] = ibex::Interval(1, 1+1e-3);
    door4[2] = ibex::Interval(1,2);

    IntervalVector wall(3);
    wall[0] = ibex::Interval(0, 1e-3);
    wall[1] = ibex::Interval(0, 2);
    wall[2] = ibex::Interval(0,2);

    C_Polyhedron ph_box1 = iv_2_polyhedron(box1);
    C_Polyhedron ph_box2 = iv_2_polyhedron(box2);
    C_Polyhedron ph_box3 = iv_2_polyhedron(box3);
    C_Polyhedron ph_box4 = iv_2_polyhedron(box4);
    C_Polyhedron ph_door1 = iv_2_polyhedron(door1);
    C_Polyhedron ph_door2 = iv_2_polyhedron(door2);
    C_Polyhedron ph_door3 = iv_2_polyhedron(door3);
    C_Polyhedron ph_door4 = iv_2_polyhedron(door4);
    C_Polyhedron ph_wall = iv_2_polyhedron(wall);

    write_VTK(ph_box1, "box1");
    write_VTK(ph_box2, "box2");
    write_VTK(ph_box3, "box3");
    write_VTK(ph_box4, "box4");
    write_VTK(ph_door1, "door1");
    write_VTK(ph_door2, "door2");
    write_VTK(ph_door3, "door3");
    write_VTK(ph_door4, "door4");
    write_VTK(ph_wall, "wall");

    plot_trajectory();
    plot_vf(box1, "box1_vf.vtp");
    plot_vf(box2, "box2_vf.vtp");
    plot_vf(box3, "box3_vf.vtp");
    plot_vf(box4, "box4_vf.vtp");
}

int main(){
    //    int dimension = 3;

    //    IntervalVector vec(dimension);
    //    vec[0] = ibex::Interval(-0.3, 0.3);
    //    vec[1] = ibex::Interval(-0.3, 0.3);
    //    vec[2] = ibex::Interval(1);

    //    IntervalVector face_in(3);
    //    face_in[0] = ibex::Interval(-1, 1);
    //    face_in[1] = ibex::Interval(-1, 1);
    //    face_in[2] = ibex::Interval(-1);
    //    C_Polyhedron ph_in(iv_2_box(face_in));
    //    C_Polyhedron ph_projection = add_ray(vec, ph_in);
    //    IntervalVector face_out(3);
    //    face_out[0] = ibex::Interval(-1, 1);
    //    face_out[1] = ibex::Interval(-1, 1);
    //    face_out[2] = ibex::Interval(1);
    //    C_Polyhedron ph_out(iv_2_box(face_out));
    //    test();
    //    test2();
    //    PPL::C_Polyhedron ph3(3);
    //    PPL::C_Polyhedron ph4(3);
    //    ph3.intersection_assign(ph4);

    //    std::cout.precision(30);
    //    mpq_class t(500.123540000000000);
    //    cout << t.get_d() << endl;
    //    cout << t.get_num().get_d() << endl;
    //    cout << t.get_den().get_d() << endl;
    //    cout << t.get_num().get_d() / t.get_den().get_d() << endl;
    //    mpq_class t1(500.12354);
    //    cout << t1.get_d() << endl;
    //    cout << t1.get_num().get_d() << endl;
    //    cout << t1.get_den().get_d() << endl;
    //    cout << t.get_num().get_d() / t.get_den().get_d() << endl;


    //    cout << ph_out.space_dimension() << endl;
    //    cout << ph_out.affine_dimension() << endl;
    //    cout << ph_out.is_discrete() << endl;
    //    cout << ph_out.is_empty() << endl;
    //    cout << ph_out.is_topologically_closed() << endl;

    //    write_VTK(ph_in, "ph_in");
    //    write_VTK(ph_out, "ph_out");
    //    write_VTK(ph_projection, "ph_projection");


    //    ibex::Variable x(2), a, b, c;
    //    Function f= Function(x, a, b, c, a*pow(x[0], 2)+b*x[0]+c-x[1]);
    //    IntervalVector test(5);
    //    test[0] = ibex::Interval(-10, 10);
    //    test[1] = ibex::Interval(-10, 10);
    //    test[2] = ibex::Interval(1);
    //    test[3] = ibex::Interval(1);
    //    test[4] = ibex::Interval(1);
    //    cout << "test = " << test << endl;
    //    ibex::CtcFwdBwd ctc(f);
    //    ctc.contract(test);
    //    cout << "test = " << test << endl;

    //    invariant::ResultStorageIBEX r(2, 1);
    //    ibex::IntervalVector iv_test(2, 1);
    //    iv_test[0] = ibex::Interval(0, 1);
    //    iv_test[1] = ibex::Interval(2, 3);
    //    r.push_back_input(iv_test, 1, 0, 0, 1, 0);


    //    invariant::ResultStoragePPL r2(2, 1);
    //    Parma_Polyhedra_Library::C_Polyhedron p(2);
    //    cout << p.is_universe() << endl;
    //    r2.push_back_input(p, 1, 0, 0, 1, 0);
    //    cout << "test" << endl;

//    plot_bisection_example();
    plot_helicoidal_example();

    return 0;
}


