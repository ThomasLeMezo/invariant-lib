#include <iostream>
#include <ppl.hh>
#include <ibex.h>
#include <iostream>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkDelaunay3D.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkCellArray.h>
#include <vtkDataSetSurfaceFilter.h>

using namespace std;
namespace PPL = Parma_Polyhedra_Library;
using namespace Parma_Polyhedra_Library::IO_Operators;
using namespace ibex;

#define IBEX_PPL_PRECISION 1e16

PPL::Rational_Box iv_2_box(const ibex::IntervalVector &iv){
    Rational_Box box(iv.size());
    for(int i=0; i<box.space_dimension(); i++){
        PPL::Variable x(i);

        if(!iv[i].is_empty()){
            box.add_constraint(x >= floor(iv[i].lb()*IBEX_PPL_PRECISION));
            box.add_constraint(x <= ceil(iv[i].ub()*IBEX_PPL_PRECISION));
        }
    }
    return box;
}

std::vector<PPL::Rational_Box> iv_2_box(const std::vector<ibex::IntervalVector> &list){
    std::vector<PPL::Rational_Box> list_RB;
    for(auto &iv:list){
        list_RB.push_back(iv_2_box(iv));
    }
    return list_RB;
}

void recursive_linear_expression_from_iv(const ibex::IntervalVector &theta,
                                         int dim,
                                         std::vector<Linear_Expression> &linear_expression_list,
                                         Linear_Expression &local_linear_expression){
    if(dim > 0){
        PPL::Variable x(dim-1);
        Linear_Expression l_b = local_linear_expression;
        Linear_Expression l_u = local_linear_expression;

        // ToDo: case theta[dim] -> lb=+oo | ub=-oo
        if(std::isinf(theta[dim-1].ub()))
            linear_expression_list.push_back(Linear_Expression(x));
        else
            l_u += x*ceil(theta[dim-1].ub()*IBEX_PPL_PRECISION);

        if(std::isinf(theta[dim-1].lb()))
            linear_expression_list.push_back(Linear_Expression(-x));
        else
            l_b += x*floor(theta[dim-1].lb()*IBEX_PPL_PRECISION);

        recursive_linear_expression_from_iv(theta, dim-1, linear_expression_list, l_u);
        recursive_linear_expression_from_iv(theta, dim-1, linear_expression_list, l_b);
    }
    else{
        linear_expression_list.push_back(local_linear_expression);
    }
}

PPL::C_Polyhedron add_ray(const ibex::IntervalVector &theta, const PPL::C_Polyhedron &p){
    Linear_Expression e = Linear_Expression(0);
    std::vector<Linear_Expression> linear_expression_list;
    PPL::C_Polyhedron p_copy(p);

    recursive_linear_expression_from_iv(theta, theta.size(), linear_expression_list,e);
    for(auto &l:linear_expression_list){
        p_copy.add_generator(ray(l));
    }
    return p_copy;
}

void write_VTK(const std::vector<Parma_Polyhedra_Library::C_Polyhedron> &ph_list, string filename){
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer< vtkPoints >::New();

    for(auto &ph:ph_list){
        for(auto &g:ph.generators()){
            if(g.is_point()){
                std::vector<double> coord;
                if(ph.space_dimension()<3){
                    for(int cpt = 0; cpt < 2; cpt++){
                        coord.clear();
                        for(int i=0; i<3; i++){
                            PPL::Variable x(i);
                            if(i<2){
                                cout << g.coefficient(x).get_d()/(g.divisor().get_d()*IBEX_PPL_PRECISION) << ",";
                                coord.push_back(g.coefficient(x).get_d()/(g.divisor().get_d()*IBEX_PPL_PRECISION));
                            }
                            else{
                                cout << cpt << endl;
                                coord.push_back(cpt/100.0);
                            }
                        }
                        points->InsertNextPoint(coord[0], coord[1], coord[2]);
                    }
                }
                else{
                    for(int i=0; i<3; i++){
                        PPL::Variable x(i);
                        if(g.space_dimension()>i){
                            coord.push_back(g.coefficient(x).get_d()/(g.divisor().get_d()*IBEX_PPL_PRECISION));
                        }
                        else{
                            coord.push_back(0.0);
                        }
                    }
//                 cout << "coord=" << coord[0] << ", " << coord[1] << ", " << coord[2] << endl;
                    points->InsertNextPoint(coord[0], coord[1], coord[2]);
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

int main(){
    int dimension = 3;

    IntervalVector pave(dimension);
    pave[0] = ibex::Interval(-1, 1);
    pave[1] = ibex::Interval(-1, 1);
    pave[2] = ibex::Interval(-1, 1);

    IntervalVector vec(dimension);
    vec[0] = ibex::Interval(-0.3, 0.3);
    vec[0] = ibex::Interval(-0.3, 0.3);
    vec[1] = ibex::Interval(1);

    C_Polyhedron ph_in(dimension, PPL::EMPTY);
    PPL::Variable x(0), y(1), z(2);
    //    ph_in.add_generator(PPL::point(0.5*IBEX_PPL_PRECISION*x+0*y-IBEX_PPL_PRECISION*z));
    //    ph_in.add_generator(PPL::point(0*x+0.5*IBEX_PPL_PRECISION*y-IBEX_PPL_PRECISION*z));
    //    ph_in.add_generator(PPL::point(-0.5*IBEX_PPL_PRECISION*x+0*y-IBEX_PPL_PRECISION*z));
    //    ph_in.add_generator(PPL::point(0*x-0.5*IBEX_PPL_PRECISION*y-IBEX_PPL_PRECISION*z));
    //    ph_in.add_generator(PPL::point(0.7*IBEX_PPL_PRECISION*x+0.7*IBEX_PPL_PRECISION*y-IBEX_PPL_PRECISION*z));

    ph_in.add_generator(PPL::point(-0.1*IBEX_PPL_PRECISION*x -1.0*IBEX_PPL_PRECISION*y));
    ph_in.add_generator(PPL::point(0.1*IBEX_PPL_PRECISION*x -1.0*IBEX_PPL_PRECISION*y));

    //    cout << "ph_in = " << ph_in.generators() << endl;

    //    IntervalVector face_in(3);
    //    face_in[0] = ibex::Interval(-0.5, 0.5);
    //    face_in[1] = ibex::Interval(-0.5, 0.5);
    //    face_in[2] = ibex::Interval(-1);
    //    C_Polyhedron ph_in(iv_2_box(face_in));

    C_Polyhedron ph_projection = add_ray(vec, ph_in);

//    std::vector<Parma_Polyhedra_Library::C_Polyhedron> ph_out_list;
//    for(auto &ph_face:rb_list){
//        C_Polyhedron ph_tmp(ph_projection);
//        C_Polyhedron ph_face_tmp(ph_face);
//        ph_tmp.intersection_assign(ph_face_tmp);
//        if(!ph_tmp.is_empty()){
//            ph_out_list.push_back(ph_tmp);
//        }
//    }
//    write_VTK(ph_out_list, "pave_out");

    cout << "matrix" << endl;

    ibex::IntervalMatrix test(3, 4);
    test[1][2] = ibex::Interval(1, 10);
    cout << test << endl;
    cout << test[0] << endl;

    return 0;
}

