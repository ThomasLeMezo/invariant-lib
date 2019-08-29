#include "language.h"

#include <omp.h>

#include "smartSubPaving.h"
#include "domain.h"
#include "dynamicsFunction.h"
#include "maze.h"
#include "vibesMaze.h"
#include "ibex_SepFwdBwd.h"

#include "vibes/vibes.h"
#include "graphiz_graph.h"
using namespace std;

namespace invariant {

int largest_positive_invariant(ibex::IntervalVector &space, ibex::Function *f_dyn, size_t nb_steps, string file_name, ibex::Function *f_dom){
    // ****** Domain ******* //
    invariant::SmartSubPaving<> subpaving(space);

    invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
    dom_outer.set_border_path_in(true);
    dom_outer.set_border_path_out(false);

    invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
    dom_inner.set_border_path_in(false);
    dom_inner.set_border_path_out(true);

    ibex::SepFwdBwd *sep_outer=nullptr, *sep_inner=nullptr;
    if(f_dom!=nullptr){
        sep_outer = new ibex::SepFwdBwd(*f_dom, ibex::GEQ); // LT, LEQ, EQ, GEQ, GT
        dom_outer.set_sep(sep_outer);
        sep_inner = new ibex::SepFwdBwd(*f_dom, ibex::LEQ); // LT, LEQ, EQ, GEQ, GT
        dom_inner.set_sep(sep_inner);
    }

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
    VibesMaze v_maze("file_name", &maze_outer, &maze_inner);
    v_maze.setProperties(0, 0, 1000, 800);
    v_maze.set_enable_cone(false);
    v_maze.drawBox(space, "white[white]");
    v_maze.show();
    v_maze.saveImage();
    v_maze.saveImage("/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/", ".svg");
    vibes::endDrawing();

    if(f_dom!=nullptr){
        delete(sep_outer);
        delete(sep_inner);
    }
    return 0;
}

}
