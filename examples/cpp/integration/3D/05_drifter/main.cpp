#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"

#include "ibex_SepFwdBwd.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

#include "vtkMaze3D.h"
#include "vtkmazeppl.h"

using namespace std;
using namespace ibex;
using namespace invariant;

const double rho =  1025.0;
const double g =  9.81;
const double m =  8.800;
const double diam_collerette =  0.24;
const double compressibility_tick =  20.0;
const double screw_thread =  1.75e-3;
const double tick_per_turn =  48;
const double piston_diameter =  0.05;
const double piston_ref_eq =  2100;

double depth = 0.0;
double piston_position = 0.0;
double piston_variation = 0.0;

const double A = g*rho/m;
const double Cf = M_PI*pow(diam_collerette/2., 2);
const double B = 0.5*rho*Cf/m;
const double tick_to_volume = (screw_thread/tick_per_turn)*pow(piston_diameter/2.0, 2)*M_PI;
//const double alpha = compressibility_tick*tick_to_volume;
const double alpha = 0.0;

int main(int argc, char *argv[]){
    // Parameters
    ibex::Interval beta = 0.02*ibex::Interval::HALF_PI;
    ibex::Interval set_point = ibex::Interval(0);
    double l1 = 0.4;
    double l2 = 0.4;

    ibex::Variable x1, x2, x3;

//    y(0) = -coeff_A*(x(2)+x(3)-coeff_compressibility*x(1))-coeff_B*copysign(x(0)*x(0), x(0));

//    double e = set_point-x2;
//    double y = x1-beta*atan(e);
//    double dx1 = -A*(x3+x4-alpha*x2)-B*abs(x1)*x1;
//    double D = 1+pow(e,2);
//    double dy = dx1 + beta*x1/D;
//    double u = (l1*dy+l2*y -beta*(2*e*pow(x1,2)-dx1*D)/pow(D,2)-2*B*abs(x1)*dx1)/A +alpha*x1;

    Function f_e(x2, (set_point - x2));
    Function f_y(x1, x2, (x1-beta*atan(f_e(x2))));
    Function f_dx1(x1, x2, x3, (-A*(x3-alpha*x2)-B*abs(x1)*x1));
    Function f_D(x2, (1+(f_e(x2)*f_e(x2))));
    Function f_dy(x1, x2,x3, (f_dx1(x1,x2,x3) + beta*x1/f_D(f_e(x2))));

    Function f_u(x1, x2, x3, min(max((alpha*x1 + (l1*f_dy(x1, x2, x3)+l2*f_y(x1, x2)
                                          -beta*(2*f_e(x2)*(x1*x1)-f_dx1(x1, x2, x3)*f_D(x2))/(f_D(x2)*f_D(x2))
                                          -2*B*abs(x1)*f_dx1(x1, x2, x3))/A), -10*tick_to_volume), 10*tick_to_volume));

    Function f1(x1, x2, x3, (-A*(x3-alpha*x2)-B*sign(x1)*x1*x1));

    Function f(x1, x2, x3, -Return(f1(x1, x2, x3),
                                  x1,
                                  f_u(x1, x2, x3)));

    // Test Function
//    Function f(x1, x2, x3, -Return(-x1,
//                                  -x2,
//                                  -x3));

    ibex::IntervalVector space(3);
    space[0] = ibex::Interval(-0.1,0.1); // Speed
    space[1] = ibex::Interval(-0.1,0.1); // Depth
    space[2] = ibex::Interval(-400.*tick_to_volume, 400.*tick_to_volume); // Volume

    cout << f.eval_vector(space) << endl;

    invariant::SmartSubPavingPPL paving(space);

    // ****** Domain Outer ******* //
    invariant::DomainPPL dom_outer(&paving, FULL_WALL);

    ibex::IntervalVector box_basin(3);
    box_basin[0] = ibex::Interval(-0.02, 0.02);
    box_basin[1] = ibex::Interval(-0.01, 0.01);
    box_basin[2] = ibex::Interval(-10.0*tick_to_volume, 10.0*tick_to_volume);

    Function f_id("x[3]", "(x[0], x[1], x[2])");
    SepFwdBwd s_outer(f_id, box_basin);
    dom_outer.set_sep_input(&s_outer);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Domain Inner ******* //
//    invariant::Domain<> dom_inner(&paving, FULL_DOOR);

//    Function f_id_inner("x[3]", "(x[0], x[1], x[2])");
//    SepFwdBwd s_inner(f_id_inner, box_basin); // LT, LEQ, EQ, GEQ, GT
//    dom_inner.set_sep(&s_inner);

//    dom_inner.set_border_path_in(true);
//    dom_inner.set_border_path_out(true);

    // ****** Dynamics Outer & Inner ******* //

    DynamicsFunction dyn_outer(&f, FWD);
//    DynamicsFunction dyn_inner(&f, FWD);

    // ******* Mazes ********* //
    invariant::MazePPL maze_outer(&dom_outer, &dyn_outer);
//    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);
    maze_outer.set_widening_limit(1);
    maze_outer.set_contraction_limit(5);
    maze_outer.set_enable_contraction_limit(true);
//    maze_outer.set_enable_contract_vector_field(true);

    vector<double> bisection_ratio = {1./space[0].diam(), 4./space[1].diam(), 1./space[2].diam()};
    paving.set_ratio_bisection(bisection_ratio);

    // ******* Algorithm ********* //

    VtkMazePPL vtkMazePPL("drifter_PPL_3D");

    double time_start = omp_get_wtime();
    for(int i=0; i<14; i++)
        paving.bisect();
    for(int i=0; i<15; i++){
        cout << i << endl;
        paving.bisect();
//        maze_outer.contract();

//        maze_outer.get_domain()->set_init(FULL_WALL);
//        maze_outer.set_enable_contract_domain(true);
        maze_outer.contract();

//        maze_outer.get_domain()->set_init(FULL_DOOR);
//        maze_outer.reset_nb_operations();
//        maze_outer.set_enable_contract_domain(false);
//        maze_outer.contract(1000);

        vtkMazePPL.show_subpaving(&maze_outer, "paving");
        vtkMazePPL.show_maze(&maze_outer, "outer");

    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

//    VtkMaze3D vtkMaze3D("dirfter_3D");
//    vtkMaze3D.show_graph(&paving);
//    vtkMaze3D.show_maze(&maze_outer, "outer");
//    vtkMaze3D.show_maze(&maze_inner, "inner");

}
