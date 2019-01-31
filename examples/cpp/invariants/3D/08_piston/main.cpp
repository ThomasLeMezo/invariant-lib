#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>
#include "vtkmazeppl.h"

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkVertex.h>

using namespace std;
using namespace ibex;
using namespace invariant;

void write_vector_field(ibex::Function *f, const ibex::IntervalVector &space, const size_t &n, const string filename){

    size_t n_axis[3] = {n, n ,n};
    size_t nb_points =1;
    for(size_t i=0; i<3; i++){
        if(space[i].is_degenerated())
            n_axis[i] = 1;
        nb_points *= n_axis[i];
    }
    cout << "Nb_points = " << nb_points << endl;

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(nb_points);

    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vertices->SetNumberOfCells(nb_points);

    vtkSmartPointer<vtkDoubleArray> u = vtkSmartPointer<vtkDoubleArray>::New();
    u->SetName("u");
    u->SetNumberOfComponents(3);
    u->SetNumberOfTuples(nb_points);

    size_t k=0;
    double x = space[0].lb();
    for(size_t nb_x=0; nb_x<n_axis[0]; nb_x++){
        double y = space[1].lb();
        for(size_t nb_y=0; nb_y<n_axis[1]; nb_y++){
            double z = space[2].lb();
            for(size_t nb_z=0; nb_z<n_axis[2]; nb_z++){
                points->SetPoint(k, x, y*10, z*10);

                vtkSmartPointer<vtkVertex> vertex = vtkSmartPointer<vtkVertex>::New();
                vertex->GetPointIds()->SetId(0, k);
                vertices->InsertNextCell(vertex);

                ibex::IntervalVector X(3);
                X[0] = ibex::Interval(x);
                X[1] = ibex::Interval(y);
                X[2] = ibex::Interval(z);

                ibex::IntervalVector vec = f->eval_vector(X);
//                cout << vec << endl;
                double factor = 1/(pow(vec[0].mid(), 2) + pow(vec[1].mid(), 2) + pow(vec[2].mid(), 2));
                u->SetTuple3(k, factor*vec[0].mid(), factor*vec[1].mid(), factor*vec[2].mid());
                k++;
                z += space[2].diam()/n_axis[2];
            }
            y += space[1].diam()/n_axis[1];
        }
        x += space[0].diam()/n_axis[0];
    }
    cout << k << endl;
    cout << space[2] << endl;
    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetVerts(vertices);
    polydata->GetPointData()->SetVectors(u);

    vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
    writer->SetFileName(filename.c_str());
    writer->SetInputData(polydata);
    writer->Write ();
}


int main(int argc, char *argv[])
{
    ibex::Variable x1, x2, x3;

    IntervalVector space(3);
    space[0] = ibex::Interval(-0.1, 0.1); // Velocity
    space[1] = ibex::Interval(0.3, 0.5); // Position
    space[2] = -1.7e-4/2.0*ibex::Interval(-1, 1); // Piston volume (m3)

    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);

#if 0
    invariant::DomainPPL dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);
#else

    invariant::DomainPPL dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    IntervalVector init(3);
    init[0] = ibex::Interval(-1e-3, 1e-3); // Velocity
    init[1] = 0.4+ibex::Interval(1e-3, 1e-3); // Position
    init[2] = 1e-3*-1.7e-4/2.0*ibex::Interval(-1, 1); // Piston volume (m3)
    Function f_sep(x1, x2, x3, Return(x1, x2, x3));
    SepFwdBwd s(f_sep, init); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);
#endif

    // ****** Dynamics ******* //

    // Physical parameters
    ibex::Interval g(9.81);
    ibex::Interval rho(1025.0);
    ibex::Interval m(8.8);
    ibex::Interval diam(0.24);
    ibex::Interval alpha(1.432e-06);

    // Regulation parameters
    ibex::Interval x2_target(0.4); // Desired depth
    ibex::Interval beta(0.04/ibex::Interval::HALF_PI);
    ibex::Interval l1(0.1);
    ibex::Interval l2(0.1);

    ibex::Interval Cf(ibex::Interval::PI*pow(diam/2.0,2));

    // Command
    ibex::Interval A(g*rho/m);
    ibex::Interval B(0.5*rho*Cf/m);

    ibex::Function y(x1, x2, (x1+beta*atan(x2_target-x2)));
    ibex::Function dx1(x1, x2, x3, (-A*(x3-alpha*x2)-B*abs(x1)*x1));
    ibex::Function e(x2, (x2_target-x2));
    ibex::Function D(x2, (1+pow(e(x2), 2)));
    ibex::Function dy(x1, x2, x3, (dx1(x1, x2, x3)-beta*x1/D(x2)));

    ibex::Function u(x1, x2, x3, (1.0/A*(l1*dy(x1, x2, x3)+l2*y(x1, x2)+beta*(2*e(x2)*pow(x1,2)-dx1(x1, x2, x3)*D(x2))/(pow(D(x2),2))-2*B*abs(x1)*x1)+alpha*x1));

    // Evolution function
    ibex::Function f(x1, x2, x3, Return(-A*(x3-alpha*x2)-B*abs(x1)*x1,
                                        x1,
                                        u(x1, x2, x3)));

//    DynamicsFunction dyn(&f, FWD_BWD);
    DynamicsFunction dyn(&f, FWD);

    // Test
//    ibex::IntervalVector iv_test(3);
//    iv_test[0] = ibex::Interval(-0.01, 0.01); // Velocity
//    iv_test[1] = ibex::Interval(0.0, 0.02); // Position
//    iv_test[2] = ibex::Interval(-1e-6, 1e-6); // Volume
//    cout << f.eval_vector(iv_test) << endl;

//    write_vector_field(&f, iv_test, 5, "vector_field.vtk");

    // ******* Maze ********* //
    invariant::MazePPL maze_outer(&dom, &dyn);
//    invariant::MazePPL maze_inner(&dom, &dyn);
//    maze_outer.set_enable_contraction_limit(true);
//    maze_outer.set_contraction_limit(5);
    maze_outer.set_widening_limit(20);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    VtkMazePPL vtkMazePPL("Piston");

    for(int i=0; i<18; i++){
        cout << i << endl;
        paving.bisect();
//        maze_outer.contract(5*paving.size_active());

        maze_outer.get_domain()->set_init(FULL_WALL);
        maze_outer.set_enable_contract_domain(true);
        cout << " ==> Outer widening" << endl;
        maze_outer.contract();

        maze_outer.get_domain()->set_init(FULL_DOOR);
        maze_outer.reset_nb_operations();
        maze_outer.set_enable_contract_domain(false);
        cout << " ==> Outer back" << endl;
        maze_outer.contract(30000);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

//    IntervalVector position_info(3);
//    position_info[0] = ibex::Interval(0.5);
//    position_info[1] = ibex::Interval(0.5);
//    position_info[2] = ibex::Interval(0.2);
//    vtkMaze3D.show_room_info(&maze, position_info);
}
