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

int main(int argc, char *argv[]){

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
    double kt = 10.0;

    double kd = 10.0;
    double L = 2.5; // distance between pendulum
    double d0 = 1.5;

    // ****** Dynamics ******* //
    ibex::Variable t1, t2, dt1, dt2;
    ibex::Function x1(t1, l*cos(t1));
    ibex::Function y1(t1, l*sin(t1));
    ibex::Function x2(t2, l*cos(t2));
    ibex::Function y2(t2, l*sin(t2)+L);

    ibex::Function d(t1,t2, sqrt(pow(x2(t2)-x1(t1),2)+pow(y2(t2)-y1(t1),2)));
    ibex::Function gamma1(t1,t2,atan2(y2(t2)-y1(t1),x2(t2)-x1(t1)));
    ibex::Function gamma2(t1,t2,atan2(y1(t1)-y2(t2),x1(t1)-x2(t2)));

    ibex::Function df1(t1, t2, dt1, g*sin(t1)+kd*(d(t1,t2)-d0)*sin(gamma1(t1,t2)-t1)-kt*dt1);
    ibex::Function df2(t1, t2, dt2, g*sin(t2)+kd*(d(t1,t2)-d0)*sin(gamma2(t1,t2)-t2)-kt*dt2);

    ibex::Function f1(dt1, t1, t2, Return(t1,
                                          df1(t1,t2,dt1)));
    DynamicsInclusionFunction dyn1(&f1, ibex::IntervalVector(1, space[0]), FWD_BWD);

    ibex::Function f2(dt2, t2, t1, Return(t2,
                                          df2(t1,t2,dt2)));
    DynamicsInclusionFunction dyn2(&f2, ibex::IntervalVector(1, space[0]), FWD_BWD);

    // ******* Maze ********* //
    invariant::MazeIBEX maze1(&dom1, &dyn1);
    invariant::MazeIBEX maze2(&dom2, &dyn2);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

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

        while(bounding_box_1_old != bounding_box_1 && bounding_box_2_old != bounding_box_2 && step <5){
            cout << "step = " << step << endl;
            step++;
            maze1.contract();

            bounding_box_1 = maze1.get_bounding_box();
            cout << "theta1 = " << bounding_box_1[0] << endl;
            dyn2.set_inclusion_parameter(bounding_box_1[0]);
            maze2.compute_vector_field();
            maze2.contract();

            bounding_box_2 = maze2.get_bounding_box();
            cout << "theta2 = " << bounding_box_2[0] << endl;
            dyn1.set_inclusion_parameter(bounding_box_2[0]);
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
