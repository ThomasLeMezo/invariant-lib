#include "language.h"

#include <omp.h>

#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"
#include "ibex_Sep.h"
#include "ibex_SepFwdBwd.h"

#include "vibes/vibes.h"
#include "graphiz_graph.h"
#include "vtkmazeppl.h"
using namespace std;

namespace invariant {

// To improve...

void save_maze_image(invariant::MazeIBEX *maze_outer, invariant::MazeIBEX *maze_inner, std::string directory, std::string file_name){
    std::vector<invariant::MazeIBEX*> list_outer{maze_outer};
    std::vector<invariant::MazeIBEX*> list_inner{maze_inner};
    save_maze_image(list_outer,list_inner,directory, file_name);
}

void save_maze_image(std::vector<invariant::MazeIBEX*> &maze_outer, std::vector<invariant::MazeIBEX*> &maze_inner, std::string directory, std::string file_name){
    VibesMaze v_maze(file_name, maze_outer, maze_inner);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);

    v_maze.set_ipe_ratio(100,80,false);

    ibex::IntervalVector bounding_box = maze_outer[0]->get_bounding_box();

    v_maze.set_axis_limits(bounding_box[0].lb(), bounding_box[0].diam()/4., bounding_box[1].lb(), bounding_box[1].diam()/4.);
    v_maze.set_enable_white_boundary(false);
    v_maze.set_thickness_pen_factor(1e-4);
    v_maze.set_enable_vibes(true);
    v_maze.set_number_digits_x(1);
    v_maze.set_number_digits_y(1);
    v_maze.show();

    v_maze.draw_axis("x_1", "x_2");

    v_maze.saveIpe(directory);
}

void save_maze_image(invariant::MazeIBEX *maze_outer, std::string directory, std::string file_name){
    VibesMaze v_maze(file_name, maze_outer);
    v_maze.setProperties(0, 0, 1024, 1024);
    v_maze.set_enable_cone(false);

    v_maze.set_ipe_ratio(100,80,false);

    ibex::IntervalVector bounding_box = maze_outer->get_bounding_box();

    v_maze.set_axis_limits(bounding_box[0].lb(), bounding_box[0].diam()/4., bounding_box[1].lb(), bounding_box[1].diam()/4.);
    v_maze.set_enable_white_boundary(false);
    v_maze.set_thickness_pen_factor(1e-4);
    v_maze.set_enable_vibes(true);
    v_maze.set_number_digits_x(1);
    v_maze.set_number_digits_y(1);
    v_maze.show();

    v_maze.draw_axis("x_1", "x_2");

    v_maze.saveIpe(directory);
}


int invariant_PPL(ibex::IntervalVector &space, ibex::Function *f_dyn, size_t nb_steps, string file_name, size_t contraction_limit){
  // ****** Domain ******* //
  invariant::SmartSubPavingPPL paving(space);
  invariant::DomainPPL dom(&paving, FULL_DOOR);

  dom.set_border_path_in(false);
  dom.set_border_path_out(false);

  DynamicsFunction dyn(f_dyn, FWD_BWD);

  // ******* Maze ********* //
  invariant::MazePPL maze(&dom, &dyn);

  maze.set_enable_contraction_limit(true);
  maze.set_contraction_limit(contraction_limit);

  // ******* Algorithm ********* //
  double time_start = omp_get_wtime();
  for(size_t i=0; i<nb_steps; i++){
      cout << i << endl;
      paving.bisect();
      maze.contract();
  }
  cout << "TIME = " << omp_get_wtime() - time_start << endl;

  cout << paving << endl;

  VtkMazePPL vtkMazePPL(file_name);
  vtkMazePPL.show_maze(&maze);

  return 0;
}

int largest_positive_invariant(ibex::IntervalVector &space, ibex::Function *f_dyn, size_t nb_steps, string file_name, ibex::Function *f_dom){
    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(true);

    ibex::SepFwdBwd sep_outer = ibex::SepFwdBwd(*f_dom, ibex::GEQ);
    dom_outer.set_sep(&sep_outer);
    ibex::SepFwdBwd sep_inner = ibex::SepFwdBwd(*f_dom, ibex::LEQ);
    dom_inner.set_sep(&sep_inner);

    // ****** Dynamics ******* //
    invariant::DynamicsFunction dyn_outer(f_dyn, BWD);
    invariant::DynamicsFunction dyn_inner(f_dyn, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(size_t i=0; i<nb_steps; i++){
     cout << i << endl;

     subpaving.bisect();
     maze_outer.contract();
     maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::beginDrawing();
    VibesMaze v_maze(file_name, &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
//    v_maze.drawBox(space, "white[white]");
//    v_maze.show();

    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

    save_maze_image(&maze_outer, &maze_inner, "/home/lemezoth/Pictures/", file_name);

    vibes::endDrawing();

    return 0;
}

int largest_positive_invariant(ibex::IntervalVector &space, ibex::Function *f_dyn, size_t nb_steps, string file_name){
    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(true);

    // ****** Dynamics ******* //
    invariant::DynamicsFunction dyn_outer(f_dyn, BWD);
    invariant::DynamicsFunction dyn_inner(f_dyn, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(size_t i=0; i<nb_steps; i++){
     cout << i << endl;

     subpaving.bisect();
     maze_outer.contract();
     maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::beginDrawing();
//    VibesMaze v_maze(file_name, &maze_outer, &maze_inner);
//    v_maze.setProperties(0, 0, 1000, 800);
//    v_maze.set_enable_cone(false);
//    v_maze.drawBox(space, "white[white]");
//    v_maze.show();
//    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");

    save_maze_image(&maze_outer, &maze_inner, "/home/lemezoth/Pictures/", file_name);
    vibes::endDrawing();

    return 0;
}

int largest_invariant(ibex::IntervalVector &space, ibex::Function *f_dyn_positive, ibex::Function *f_dyn_negative, size_t nb_steps, string file_name){
    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner_positive(&subpaving, FULL_WALL);
    dom_inner_positive.set_border_path_in(true);
    dom_inner_positive.set_border_path_out(true);

    invariant::Domain<> dom_inner_negative(&subpaving, FULL_WALL);
    dom_inner_negative.set_border_path_in(true);
    dom_inner_negative.set_border_path_out(true);

    // ****** Dynamics ******* //
    invariant::DynamicsFunction dyn_outer(f_dyn_negative, FWD_BWD);
    invariant::DynamicsFunction dyn_inner_positive(f_dyn_positive, FWD);
    invariant::DynamicsFunction dyn_inner_negative(f_dyn_negative, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner_positive(&dom_inner_positive, &dyn_inner_positive);
    invariant::Maze<> maze_inner_negative(&dom_inner_negative, &dyn_inner_negative);

    invariant::BooleanTreeUnion<> *bisection_inner = new invariant::BooleanTreeUnion<>(&maze_inner_positive, &maze_inner_negative);
    invariant::BooleanTreeInter<> *bisection = new invariant::BooleanTreeInter<>(&maze_outer, bisection_inner);
    subpaving.set_bisection_tree(bisection); // ToDo delete objects at the end

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(size_t i=0; i<nb_steps; i++){
     cout << i << endl;
     subpaving.bisect();
     maze_outer.contract();
     maze_inner_negative.contract();
     maze_inner_positive.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vector<invariant::MazeIBEX*> list_outer{&maze_outer};
    vector<invariant::MazeIBEX*> list_inner{&maze_inner_positive, &maze_inner_negative};

    vibes::beginDrawing();
    VibesMaze v_maze(file_name, list_outer, list_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
//    v_maze.drawBox(space, "white[white]");
//    v_maze.show();

    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");
    save_maze_image(list_outer, list_inner,"/home/lemezoth/Pictures/", file_name);
    vibes::endDrawing();

    return 0;
}

int largest_positive_invariant(ibex::IntervalVector &space,
                               ibex::Function *f_dyn,
                               size_t nb_steps,
                               std::string file_name,
                               ibex::Sep* sep_outer
                               ){
    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(true);

    dom_outer.set_sep(sep_outer);
    ibex::SepNot sep_inner(*sep_outer);
    dom_inner.set_sep(&sep_inner);

    // ****** Dynamics ******* //
    invariant::DynamicsFunction dyn_outer(f_dyn, BWD);
    invariant::DynamicsFunction dyn_inner(f_dyn, BWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(size_t i=0; i<nb_steps; i++){
     cout << i << endl;

     subpaving.bisect();
     maze_outer.contract();
     maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::beginDrawing();
    VibesMaze v_maze(file_name, &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
//    v_maze.drawBox(space, "white[white]");
//    v_maze.show();

    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");
    save_maze_image(&maze_outer, &maze_inner, "/home/lemezoth/Pictures/", file_name);
    vibes::endDrawing();

    return 0;
}

int largest_positive_invariant(ibex::IntervalVector &space,
                               ibex::Function* f_outer,
                               vector<ibex::Function*> &f_inner,
                               size_t nb_steps,
                               std::string file_name,
                               ibex::Sep* sep_outer
                               ){
    // ****** Domain ******* //
//    omp_set_num_threads(1);
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(true);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    dom_outer.set_sep(sep_outer);
    ibex::SepNot sep_inner(*sep_outer);
    dom_inner.set_sep(&sep_inner);

    // ****** Dynamics ******* //
    invariant::DynamicsFunction dyn_outer(f_outer, FWD);
    invariant::DynamicsFunction dyn_inner(f_inner, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(size_t i=0; i<nb_steps; i++){
     cout << i << endl;

     subpaving.bisect();
     maze_outer.contract();
     maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::beginDrawing();
    VibesMaze v_maze(file_name, &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
//    v_maze.drawBox(space, "white[white]");
//    v_maze.show();

    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");
    save_maze_image(&maze_outer, &maze_inner, "/home/lemezoth/Pictures/", file_name);
    vibes::endDrawing();

    return 0;
}

int largest_positive_invariant(ibex::IntervalVector &space,
                               ibex::Function* f_outer,
                               vector<ibex::Function*> &f_inner,
                               size_t nb_steps,
                               std::string file_name
                               ){
    // ****** Domain ******* //
//    omp_set_num_threads(1);
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(false);
    dom_outer.set_border_path_out(true);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(true);
    dom_inner.set_border_path_out(false);

    // ****** Dynamics ******* //
    invariant::DynamicsFunction dyn_outer(f_outer, FWD);
    invariant::DynamicsFunction dyn_inner(f_inner, FWD);

    // ******* Maze ********* //
    invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
    invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

    // ******* Algorithm ********* //
    double time_start = omp_get_wtime();

    for(size_t i=0; i<nb_steps; i++){
     cout << i << endl;

     subpaving.bisect();
     maze_outer.contract();
     maze_inner.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    vibes::beginDrawing();
    VibesMaze v_maze(file_name, &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
//    v_maze.drawBox(space, "white[white]");
//    v_maze.show();

    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");
    save_maze_image(&maze_outer, &maze_inner, "/home/lemezoth/Pictures/", file_name);
    vibes::endDrawing();

    return 0;
}

int largest_positive_invariant_python(ibex::IntervalVector &space,
                                      ibex::Function* f_outer,
                                      size_t nb_steps,
                                      std::string file_name,
                                      std::vector<ibex::Function*> &f_inner,
                                      ibex::Sep* sep_outer){
    if(f_inner.size()==0 && sep_outer == nullptr){
        largest_positive_invariant(space, f_outer, nb_steps, file_name);
    }
    else if(f_inner.size()==0){
        largest_positive_invariant(space, f_outer, nb_steps, file_name, sep_outer);
    }
    else{
        largest_positive_invariant(space, f_outer, f_inner, nb_steps, file_name, sep_outer);
    }
    return 0;
}

}
