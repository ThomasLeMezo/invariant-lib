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
    invariant::SmartSubPavingIBEX paving(space);
    invariant::DomainIBEX dom(&paving, FULL_DOOR);

    dom.set_border_path_in(false);
    dom.set_border_path_out(false);

    ibex::Interval g(9.81);
    ibex::Interval l(1);

    // ****** Dynamics ******* //
    ibex::Function f1(x1, x2, x3, Return(x2,
                                         -x1+x3-5*x2));
    DynamicsInclusionFunction dyn1(&f1, ibex::IntervalVector(1, ibex::Interval(-1, 0)), FWD_BWD);

    ibex::Function f2(x1, x2, x3, Return(x2,
                                         -x1+x3-5*x2));
    DynamicsInclusionFunction dyn2(&f2, ibex::IntervalVector(1, ibex::Interval(0, 1)), FWD_BWD);

    ibex::Variable theta1, theta2;
    ibex::Function f_attraction(theta1, theta2, 1.0/(cos(theta1)-cos(theta2)+2.5));

    // ******* Maze ********* //
    invariant::MazeIBEX maze1(&dom, &dyn1);
    invariant::MazeIBEX maze2(&dom, &dyn2);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    IntervalVector theta(2);
    ibex::Interval force;
    //
    omp_set_num_threads(1);
    for(int i=0; i<12; i++){
        paving.bisect();
        size_t step = 0;
        cout << "bisection = " << i << endl;

        IntervalVector bounding_box_1(2);
        IntervalVector bounding_box_2(2);
        IntervalVector bounding_box_1_old = maze1.get_bounding_box();
        IntervalVector bounding_box_2_old = maze2.get_bounding_box();

        while(bounding_box_1_old != bounding_box_1 && bounding_box_2_old != bounding_box_2 && step <3){
            cout << "step = " << step << endl;
            step++;
            maze1.contract();
            bounding_box_1 = maze1.get_bounding_box();

            theta[0] = bounding_box_1[0];
            theta[1] = bounding_box_2[0];

            force = f_attraction.eval(theta);
            dyn2.set_inclusion_parameter(force);
            cout << "force = " << force << endl;
            maze2.compute_vector_field();

            maze2.contract();
            bounding_box_2 = maze2.get_bounding_box();

            theta[0] = bounding_box_1[0];
            theta[1] = bounding_box_2[0];

            force = f_attraction.eval(theta);
            dyn1.set_inclusion_parameter(-force);
            maze1.compute_vector_field();
        }
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;

    vibes::beginDrawing();
    VibesMaze v_maze1("Maze1", &maze1);
    v_maze1.setProperties(0, 0, 1024, 1024);
    v_maze1.show();
    IntervalVector bounding_box = maze1.get_bounding_box();
    v_maze1.drawBox(bounding_box, "green[]");

    VibesMaze v_maze2("Maze2", &maze2);
    v_maze2.setProperties(0, 0, 1024, 1024);
    v_maze2.show();
    bounding_box = maze2.get_bounding_box();
    v_maze2.drawBox(bounding_box, "green[]");

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(0);
    //    position_info[1] = ibex::Interval(1);
    //    v_maze.get_room_info(&maze, position_info);

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-1);
    //    position_info[1] = ibex::Interval(1);
    //    v_maze.get_room_info(&maze, position_info);
    //    v_maze.show_room_info(&maze, position_info);

    //    position_info[0] = ibex::Interval(0);
    //    position_info[1] = ibex::Interval(-1);
    //    v_maze.get_room_info(&maze, position_info);
    vibes::endDrawing();

}
