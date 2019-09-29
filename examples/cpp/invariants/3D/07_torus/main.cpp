#include "ibex_SepFwdBwd.h"
#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include "graphiz_graph.h"
#include <omp.h>
#include "vtkMaze3D.h"
#include "vtkmazeppl.h"

using namespace std;
using namespace ibex;
using namespace invariant;


void with_ppl(){
  ibex::Variable x, y, z;

  IntervalVector space(3);
  space[0] = ibex::Interval(-2, 2);
  space[1] = ibex::Interval(-2, 2);
  space[2] = ibex::Interval(-2, 2);

  // ****** Domain ******* //
  invariant::SmartSubPavingPPL paving(space);
  invariant::DomainPPL dom(&paving, FULL_DOOR);

  Function f_sep(x, y, z, pow(x, 2)+pow(y, 2)+pow(z, 2)-pow(0.5, 2));
  SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
  dom.set_sep(&s);

  dom.set_border_path_in(false);
  dom.set_border_path_out(false);

  // ****** Dynamics ******* //
  ibex::Function f(x, y, z, Return(x-(x+y)*(pow(x,2)+pow(y,2)),
                                  y+(x-y)*(pow(x,2)+pow(y,2)),
                                   -z*(1+pow(x,2)+pow(y,2))));
  DynamicsFunction dyn(&f, FWD_BWD);

  // ******* Maze ********* //
  invariant::MazePPL maze(&dom, &dyn);
//    maze.set_widening_limit(5);
  maze.set_enable_contraction_limit(true);
  maze.set_contraction_limit(25);

  // ******* Algorithm ********* //
  double time_start = omp_get_wtime();
  VtkMazePPL vtkMazePPL("torus");

  for(int i=0; i<16; i++){
      cout << i << endl;
      paving.bisect();
      maze.contract(5*paving.size_active());
      vtkMazePPL.show_maze(&maze);
  }
  cout << "TIME = " << omp_get_wtime() - time_start << endl;

  cout << paving << endl;
}

void with_box(){
  ibex::Variable x, y, z;

  IntervalVector space(3);
  space[0] = ibex::Interval(-2, 2);
  space[1] = ibex::Interval(-2, 2);
  space[2] = ibex::Interval(-2, 2);

  // ****** Domain ******* //
  invariant::SmartSubPavingIBEX paving(space);
  invariant::DomainIBEX dom(&paving, FULL_DOOR);

  Function f_sep(x, y, z, pow(x, 2)+pow(y, 2)+pow(z, 2)-pow(0.5, 2));
  SepFwdBwd s(f_sep, GEQ); // LT, LEQ, EQ, GEQ, GT
  dom.set_sep(&s);

  dom.set_border_path_in(false);
  dom.set_border_path_out(false);

  // ****** Dynamics ******* //
  ibex::Function f(x, y, z, Return(x-(x+y)*(pow(x,2)+pow(y,2)),
                                  y+(x-y)*(pow(x,2)+pow(y,2)),
                                   -z*(1+pow(x,2)+pow(y,2))));
  DynamicsFunction dyn(&f, FWD_BWD);

  // ******* Maze ********* //
  invariant::MazeIBEX maze(&dom, &dyn);

  // ******* Algorithm ********* //
  double time_start = omp_get_wtime();
  VtkMaze3D vtkMazeIBEX("torus_box");

  for(int i=0; i<25; i++){
      cout << i << endl;
      paving.bisect();
      maze.contract();
      vtkMazeIBEX.show_maze(&maze);
  }
  cout << "TIME = " << omp_get_wtime() - time_start << endl;

  cout << paving << endl;
}

int main(int argc, char *argv[]){
  with_box();
}
