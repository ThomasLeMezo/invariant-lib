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

#define COMPUTE_INV 1
#define INVARIANT_FATHER_HULL 1

int main(int argc, char *argv[]){
    omp_set_num_threads(48);

    cout << "omp_get_max_threads = " << omp_get_max_threads() << endl;

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
//    cout << "l1 = " << l1 << endl;
//    cout << "l2 = " << l2 << endl;

    double Cf = M_PI*pow(diam/2.0,2);

    // Command
    double A = g*rho/m;
    double B = 0.5*rho*Cf/m;

    //****************************************************//

    ibex::Variable x1, x2, x3;
    IntervalVector space(3);

#if COMPUTE_INV
//    space[0] = beta*atan(0.01)*ibex::Interval(-1, 1); // Velocity
    space[0] = 2e-4*ibex::Interval(-1, 1); // Velocity
    space[1] = 0.001*ibex::Interval(-1e-3, 1); // Position
    space[2] = 5*tick_to_volume*ibex::Interval(-1, 1); // Piston volume (m3)

//    space[0] = ibex::Interval(-2.98521e-5, 8.7575e-7);
//    space[1] = ibex::Interval(0.00094869, 0.000998352);
//    space[2] = ibex::Interval(-1.64115e-9, 1.11853e-8);


    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);

    invariant::DomainPPL dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);
#else
    space[0] = 0.1*ibex::Interval(-1, 1); // Velocity
    space[1] = 1.0*ibex::Interval(-1, 1); // Position
    space[2] = 300*tick_to_volume*ibex::Interval(-1, 1); // Piston volume (m3)

    // ****** Domain ******* //
    invariant::SmartSubPavingPPL paving(space);

    invariant::DomainPPL dom(&paving, FULL_WALL);
    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    invariant::DomainPPL dom_inner(&paving, FULL_DOOR);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(true);

    IntervalVector init(3);
    init[0] = 0.01*ibex::Interval(-1, 1); // Velocity
    init[1] = 0.1*ibex::Interval(-1, 1); // Position
    init[2] = 50*tick_to_volume*ibex::Interval(-1, 1);
    cout << "Init = " << init << endl;
    Function f_sep(x1, x2, x3, Return(x1, x2, x3));
    SepFwdBwd s(f_sep, init); // LT, LEQ, EQ, GEQ, GT
    dom.set_sep(&s);

    SepNot s_inner(s);
    dom_inner.set_sep_output(&s_inner);
#endif
    cout << "Space = " << space << endl;

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
//    VtkMazePPL vtkMazePPL_inner("Piston_inner");

#if 1
    double n_traj = 5.0;
    double t_max = 100.0;
    double dt = 0.01;
#if COMPUTE_INV
    vtkMazePPL.simu_trajectory(dyn, space, n_traj, t_max, dt, vector<double>{1.0, 1.0, 1000.0});
#else
    vtkMazePPL.simu_trajectory(dyn, init, n_traj, t_max, dt, vector<double>{1.0, 1.0, 1000.0});
#endif
#endif

#if 1
    // ******* Maze ********* //
    invariant::MazePPL maze_outer(&dom, &dyn);
    maze_outer.set_enable_contraction_limit(true);
    maze_outer.set_contraction_limit(20);
    maze_outer.set_widening_limit(20);

//    maze_outer.set_enable_father_hull(true);

//    invariant::MazePPL maze_inner(&dom_inner, &dyn);
//    maze_inner.set_enable_contraction_limit(true);
//    maze_inner.set_contraction_limit(15);
//    maze_inner.set_widening_limit(15);

    for(int i=0; i<30; i++){
        cout << i << endl;
        paving.bisect();

        // Invariant +/-
#if COMPUTE_INV
        maze_outer.contract(20*paving.size_active());

#else
        // Reach set
        maze_outer.get_domain()->set_init(FULL_WALL);
        maze_outer.set_enable_contract_domain(true);
        cout << " ==> Outer widening" << endl;
        maze_outer.contract();

//        maze_outer.get_domain()->set_init(FULL_DOOR);
//        maze_outer.reset_nb_operations();
//        maze_outer.set_enable_contract_domain(false);
//        cout << " ==> Outer back" << endl;
//        maze_outer.contract(1.1*paving.size_active());

//        maze_inner.contract(10*paving.size_active());
#endif
        vtkMazePPL.show_maze(&maze_outer);
//        vtkMazePPL_inner.show_maze(&maze_inner, "", true);
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
