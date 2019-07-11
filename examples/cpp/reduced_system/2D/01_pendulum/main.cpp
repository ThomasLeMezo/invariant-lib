#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsinclusionfunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2, x3;

    IntervalVector space(2);
    space[0] = -ibex::Interval::HALF_PI | ibex::Interval::HALF_PI;
    space[1] = -ibex::Interval::PI | ibex::Interval::PI;

    // ****** Domain ******* //
    invariant::SmartSubPavingIBEX paving1(space);
    invariant::DomainIBEX dom1(&paving1, FULL_DOOR);
    dom1.set_border_path_in(false);
    dom1.set_border_path_out(false);

    invariant::SmartSubPavingIBEX paving2(space);
    invariant::DomainIBEX dom2(&paving2, FULL_DOOR);
    dom2.set_border_path_in(false);
    dom2.set_border_path_out(false);

    double g = 9.81;
    double l = 1; // Length of pendulum
    double m = 1;
    double k = 10.0;
    double L = 2.5; // distance between pendulum
    double k_mag = 0.4;

    // ****** Dynamics ******* //
    ibex::Variable theta1, theta2;
    ibex::Function xA(theta1, l*sin(theta1));
    ibex::Function yA(theta1, -l*cos(theta1));
    ibex::Function xB(theta2, l*sin(theta2)+L);
    ibex::Function yB(theta2, -l*cos(theta2));

    ibex::Function r(theta1, theta2, pow(xB(theta2)-xA(theta1),2)+pow(yB(theta2)-yA(theta2),2));
    ibex::Function alpha_thetaA(theta1, theta2, atan2(yB(theta2)-yA(theta1),xB(theta2)-xA(theta1))+theta1);
    ibex::Function alpha_thetaB(theta1, theta2, atan2(yA(theta1)-yB(theta2),xA(theta1)-xB(theta2))+theta2);

    ibex::Function f_attractionA(theta1, theta2, k_mag*r(theta1, theta2)*cos(alpha_thetaA(theta1, theta2)));
    ibex::Function f_attractionB(theta1, theta2, k_mag*r(theta1, theta2)*cos(alpha_thetaB(theta1, theta2)));

    ibex::IntervalVector theta_bounds(2, space[0]);
    ibex::Interval force_boundsA = f_attractionA.eval(theta_bounds);
    ibex::Interval force_boundsB = f_attractionB.eval(theta_bounds);
    cout << "Force_boundsA = " << force_boundsA << endl;
    cout << "Force_boundsB = " << force_boundsB << endl;

    ibex::Function f1(x1, x2, x3, Return(x2,
                                         -(g/l)*sin(x1)-(k/m)*x2+x3));
    DynamicsInclusionFunction dyn1(&f1, ibex::IntervalVector(1, force_boundsA), FWD_BWD);

    ibex::Function f2(x1, x2, x3, Return(x2,
                                         -(g/l)*sin(x1)-(k/m)*x2+x3));
    DynamicsInclusionFunction dyn2(&f2, ibex::IntervalVector(1, force_boundsB), FWD_BWD);


//    ibex::Function f_attraction(theta1, theta2, 0.0*theta1);

    // ******* Maze ********* //
    invariant::MazeIBEX maze1(&dom1, &dyn1);
    invariant::MazeIBEX maze2(&dom2, &dyn2);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    IntervalVector theta(2);
    ibex::Interval force;
    //
//    omp_set_num_threads(1);
    for(int i=0; i<20; i++){
        paving1.bisect();
        paving2.bisect();
        size_t step = 0;
        cout << "bisection = " << i << endl;

        IntervalVector bounding_box_1(2);
        IntervalVector bounding_box_2(2);
        IntervalVector bounding_box_1_old = maze1.get_bounding_box();
        IntervalVector bounding_box_2_old = maze2.get_bounding_box();

        while(bounding_box_1_old != bounding_box_1 && bounding_box_2_old != bounding_box_2 && step <4){
            cout << "step = " << step << endl;
            step++;
            maze1.contract();

            bounding_box_1 = maze1.get_bounding_box();
            theta[0] = bounding_box_1[0];
            theta[1] = bounding_box_2[0];
            force = f_attractionB.eval(theta);
            cout << "forceB = " << force << endl;
            dyn2.set_inclusion_parameter(force);
            maze2.compute_vector_field();
            maze2.contract();

            bounding_box_2 = maze2.get_bounding_box();
            theta[0] = bounding_box_1[0];
            theta[1] = bounding_box_2[0];
            force = f_attractionA.eval(theta);
            cout << "forceA = " << force << endl;
            dyn1.set_inclusion_parameter(force);
            maze1.compute_vector_field();
        }
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::beginDrawing();
    VibesMaze v_maze1("pendulum1", &maze1);
    v_maze1.setProperties(0, 0, 1000, 800);
    v_maze1.set_enable_cone(false);
    v_maze1.show();
    IntervalVector bounding_box = maze1.get_bounding_box();
    v_maze1.drawBox(bounding_box, "green[]");
    v_maze1.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

    VibesMaze v_maze2("pendulum2", &maze2);
    v_maze2.setProperties(0, 0, 1000, 800);
    v_maze2.set_enable_cone(false);
    v_maze2.show();
    bounding_box = maze2.get_bounding_box();
    v_maze2.drawBox(bounding_box, "green[]");
    v_maze1.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(0.335);
//    position_info[1] = ibex::Interval(-0.02);
//    v_maze2.show_room_info(&maze2, position_info);

    vibes::endDrawing();

}
