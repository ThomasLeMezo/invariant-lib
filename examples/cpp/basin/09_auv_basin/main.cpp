#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include "ibex_SepFwdBwd.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

ibex::Function sm_model(){

  ibex::Variable x(4);

  double kw  =  -4.8331  ; // kw
  double kq  =  -9.7080  ; // kq
  double kz  =   7.5631  ; // kt
  double kt  = -58.7074  ; // kz

  double U0   (1.0)            ;
  double m    (2.72)         ;
  double rho  (1000.0)         ;
  double Iyy  (4723.0/1000.0)    ;
  double L    (14.0/10.0)        ;
  double Xupt (272.0/100.0)      ;
  double Zwpt (-23825.0/1000.0)  ;
  double Mqpt (-38914.0/10000.0) ;
  double Zw   (-229.0)         ;
  double Mq   (-53.0)          ;

  double ZU   (-0.0647); double TU   (-0.0377);   //   U
  double ZU2  ( 0.4467); double TU2  ( 0.3624);   //   U2
  double ZU3  (-0.8210); double TU3  (-0.4481);   //   U3
  double ZU4  ( 0.3821); double TU4  ( 0.2066);   //   U4
  double ZAU  (-0.1238); double TAU  (-0.0668);   //   AU
  double ZAU2 (-0.2689); double TAU2 (-0.1466);   //   AU2
  double ZAU3 (-0.0072); double TAU3 (-0.0040);   //   AU3

  double Zds = ZAU*U0 + ZAU2*U0*U0 + ZAU3*U0*U0*U0;
  double Mds = TAU*U0 + TAU2*U0*U0 + TAU3*U0*U0*U0;

  ibex::Function f(x, ibex::Return((1/(m-Zwpt))  *(            Zw*x[0] + (m-Xupt)*U0*x[1] + Zds*(-kw*x[0] - kq*x[1] - kz*x[2] - kt*x[3])),
                       (1/(Iyy-Mqpt))*((Xupt-Zwpt)*U0*x[0] +          Mq*x[1] + Mds*(-kw*x[0] - kq*x[1] - kz*x[2] - kt*x[3])),
                x[0] - U0*x[3],
                x[1]));
  return f;
}

int main(int argc, char *argv[]){
    ibex::IntervalVector space(4);
    double deg = 1.0/57.2;
    space[0] =  ibex::Interval(-1.0, 1.0); // z_dot
    space[1] =  ibex::Interval(-15.0*deg, 15.0*deg); // theta_dot
    space[2] = ibex::Interval(-20,20.0); // z
    space[3] = ibex::Interval(-30.0*deg,30.0*deg); // theta

//    space[0] = ibex::Interval(-2, 11);
//    space[1] = ibex::Interval(-7, 7);

    // ****** Domain ******* //
    invariant::SmartSubPaving<> paving(space);

//    ibex::IntervalVector box(4);
//    box[0] = ibex::Interval(-0.1, 0.1);
//    box[1] = ibex::Interval(-0.1, 0.1);
//    box[2] = ibex::Interval(-0.1, 0.1);
//    box[3] = ibex::Interval(-0.1, 0.1);
//    Function f_id("x[4]", "(x[0], x[1], x[2], x[3])");
//    SepFwdBwd s_outer(f_id, box);
//    SepNot s_inner(s_outer);

//    invariant::Domain<> dom_outer(&paving, FULL_WALL);
//    dom_outer.set_border_path_in(false);
//    dom_outer.set_border_path_out(false);
//    dom_outer.set_sep_output(&s_outer);

//    invariant::Domain<> dom_inner(&paving, FULL_DOOR);
//    dom_inner.set_border_path_in(true);
//    dom_inner.set_border_path_out(true);
//    dom_inner.set_sep_input(&s_inner);

//    // ****** Dynamics ******* //
    Function sm = sm_model();
//    Dynamics_Function dyn(&sm,
//                          FWD);

    //
    invariant::Domain<> dom_outer(&paving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    // ****** Dynamics ******* //
    DynamicsFunction dyn(&sm, FWD_BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn);
//    invariant::Maze<> maze_inner(&dom_inner, &dyn);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();
    
    for(int i=0; i<20; i++){
        cout << i << endl;
        paving.bisect();
        maze_outer.contract();
//        cout << i << " - " << maze_inner.contract() << " - " << paving.size() << endl;
        cout << " ==> Bounding box = " << maze_outer.get_bounding_box() << endl;
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << paving << endl;


//    vibes::beginDrawing();
//    VibesMaze v_maze("Cardioide", &maze_outer, &maze_inner);
//    v_maze.setProperties(0, 0, 1024, 1024);
//    v_maze.show();
//    v_maze.drawBox(box_remove, "g[]");

//    IntervalVector position_info(2);
//    position_info[0] = ibex::Interval(-0.4);
//    position_info[1] = ibex::Interval(1.34);
//    v_maze.get_room_info(&maze_inner, position_info);


//    ibex::Function f_test(x,atan2(x[1], x[0]));
//    ibex::IntervalVector box_test(2);
//    box_test[0] = ibex::Interval(-2, -1);
//    box_test[1] = ibex::Interval(-0.1, 0.1);
//    cout << "box_test -> " << f_test.eval_vector(box_test) << endl;

}
