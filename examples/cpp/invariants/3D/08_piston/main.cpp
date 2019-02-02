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

#define COMPUTE_INV 0

int main(int argc, char *argv[]){

    double screw_thread = 1.75e-3;
    double tick_per_turn = 48;
    double piston_diameter = 0.05;
    double tick_to_volume = (screw_thread/tick_per_turn)*pow(piston_diameter/2.0, 2)*M_PI;

    // Physical parameters
    double g = 9.81;
    double rho = 1025.0;
    double m = 9.045;
    double diam = 0.24;
    double alpha = tick_to_volume*20.0;

    // Regulation parameters
    double x2_target = 0.0; // Desired depth
    double beta = 0.04/M_PI_2;

    double r = -1.0;
    double l1 = -2.0*r;
    double l2 = pow(r, 2);
    cout << "l1 = " << l1 << endl;
    cout << "l2 = " << l2 << endl;

    double Cf = M_PI*pow(diam/2.0,2);

    // Command
    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;

    //****************************************************//

    ibex::Variable x1, x2, x3;
    IntervalVector space(3);

#if COMPUTE_INV
    space[0] = beta*atan(0.01)*ibex::Interval(-1, 1); // Velocity
    space[1] = 0.01*ibex::Interval(-1e-3, 1); // Position
    space[2] = 10*tick_to_volume*ibex::Interval(-1, 1); // Piston volume (m3)

    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);

    invariant::DomainPPL dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);
#else
    space[0] = 2*0.003*ibex::Interval(-1, 1); // Velocity
    space[1] = 0.04*ibex::Interval(-1, 1); // Position
    space[2] = 5*8e-4*1e-3*ibex::Interval(-1, 1); // Piston volume (m3)

    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);

    invariant::DomainPPL dom(&paving, FULL_WALL);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    IntervalVector init(3);
    init[0] = 0.001*ibex::Interval(-1, 1); // Velocity
    init[1] = 0.01*ibex::Interval(-1, 1); // Position
    init[2] = 10*tick_to_volume*ibex::Interval(-1, 1);
    Function f_sep(x1, x2, x3, Return(x1, x2, x3));
    SepFwdBwd s(f_sep, init); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);
#endif

    // ****** Dynamics ******* //

    ibex::Function e(x2, (x2_target-x2));
    ibex::Function y(x1, x2, (x1-beta*atan(e(x2))));
    ibex::Function dx1(x1, x2, x3, (-A*(x3-alpha*x2)-B*abs(x1)*x1));
    ibex::Function D(x2, (1+pow(e(x2), 2)));
    ibex::Function dy(x1, x2, x3, (dx1(x1, x2, x3)+beta*x1/D(x2)));

    ibex::Function u(x1, x2, x3, ((l1*dy(x1, x2, x3)+l2*y(x1, x2)
                                   +beta*(dx1(x1, x2, x3)*D(x2)+2.0*e(x2)*pow(x1,2))/(pow(D(x2),2))-2.0*B*abs(x1)*dx1(x1, x2, x3))/A+alpha*x1));

    // Evolution function
    ibex::Function f(x1, x2, x3, Return(dx1(x1, x2, x3),
                                        x1,
                                        u(x1, x2, x3)));


#if COMPUTE_INV
    DynamicsFunction dyn(&f, FWD_BWD);
#else
    DynamicsFunction dyn(&f, FWD);
#endif

    // Test
//    ibex::IntervalVector iv_test(3);
//    iv_test[0] = ibex::Interval(-0.01, 0.01); // Velocity
//    iv_test[1] = ibex::Interval(0.0, 0.02); // Position
//    iv_test[2] = ibex::Interval(-1e-6, 1e-6); // Volume
//    cout << f.eval_vector(iv_test) << endl;

//    write_vector_field(&f, iv_test, 5, "vector_field.vtk");



    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    VtkMazePPL vtkMazePPL("Piston");

#if 1
    double n_traj = 3.0;
    double t_max = 100.0;
    double dt = 0.01;
    size_t k=0;
#if COMPUTE_INV
    for(double x_traj=space[0].lb(); x_traj<=space[0].ub(); x_traj+=space[0].diam()/(n_traj-1)){
        for(double y_traj=space[1].lb(); y_traj<=space[1].ub(); y_traj+=space[1].diam()/(n_traj-1)){
            for(double z_traj=space[2].lb(); z_traj<=space[2].ub(); z_traj+=space[2].diam()/(n_traj-1)){
                vtkMazePPL.simu_trajectory(&f, vector<double>{x_traj, y_traj, z_traj}, t_max, dt, vector<double>{1.0, 1.0, 1000.0});
                cout << k++ << endl;
            }
        }
    }
#else
    for(double x_traj=init[0].lb(); x_traj<=init[0].ub(); x_traj+=init[0].diam()/(n_traj-1)){
        for(double y_traj=init[1].lb(); y_traj<=init[1].ub(); y_traj+=init[1].diam()/(n_traj-1)){
            for(double z_traj=init[2].lb(); z_traj<=init[2].ub(); z_traj+=init[2].diam()/(n_traj-1)){
                vtkMazePPL.simu_trajectory(&f, vector<double>{x_traj, y_traj, z_traj}, t_max, dt, vector<double>{1.0, 1.0, 1000.0});
                cout << k++ << endl;
            }
        }
    }
#endif
#endif

#if 1
    // ******* Maze ********* //
    invariant::MazePPL maze_outer(&dom, &dyn);
    maze_outer.set_enable_contraction_limit(true);
    maze_outer.set_contraction_limit(5);
    maze_outer.set_widening_limit(5);

    for(int i=0; i<22; i++){
        cout << i << endl;
        paving.bisect();

        // Invariant +/-
#if COMPUTE_INV
        maze_outer.contract(10*paving.size_active());

#else
        // Reach set
        maze_outer.get_domain()->set_init(FULL_WALL);
        maze_outer.set_enable_contract_domain(true);
        cout << " ==> Outer widening" << endl;
        maze_outer.contract();

        maze_outer.get_domain()->set_init(FULL_DOOR);
        maze_outer.reset_nb_operations();
        maze_outer.set_enable_contract_domain(false);
        cout << " ==> Outer back" << endl;
        maze_outer.contract(1.1*paving.size_active());
#endif
        vtkMazePPL.show_maze(&maze_outer);
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

////    IntervalVector position_info(3);
////    position_info[0] = ibex::Interval(0.1);
////    position_info[1] = ibex::Interval(0.499);
////    position_info[2] = ibex::Interval(-0.07/1e3);
////    vtkMazePPL.show_room_info(&maze_outer, position_info);

//    IntervalVector position_info(3);
//    position_info[0] = ibex::Interval(0);
//    position_info[1] = ibex::Interval(0.5);
//    position_info[2] = ibex::Interval(0.0);
////    vtkMazePPL.show_room_info(&maze_outer, position_info);

#endif
}
