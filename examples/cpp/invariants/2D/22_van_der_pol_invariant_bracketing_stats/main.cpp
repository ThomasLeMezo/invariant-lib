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

using namespace std;
using namespace ibex;
using namespace invariant;

void save_stat_to_file(string namefile, vector<array<double,5>> &memory){
    ofstream stat_file;
    stat_file.open(namefile);
    for(array<double, 5> &val:memory){
        for(size_t i=0; i<5; i++)
            stat_file << val[i] << " ";
        stat_file << endl;
    }
    stat_file.close();
}

int main(int argc, char *argv[])
{
    omp_set_num_threads(5);
    vector<array<double,5>> memory;

    for(int k=0; k<30; k++){
        cout << "****** STEP " << k << " *****" << endl;

        ibex::Variable x1, x2;

        IntervalVector space(2);
        space[0] = ibex::Interval(-3,3);
        space[1] = ibex::Interval(-3,3);

        // ****** Domain ******* //
        invariant::SmartSubPaving<> subpaving(space);

        invariant::Domain<> dom_outer(&subpaving, FULL_DOOR);
        dom_outer.set_border_path_in(false);
        dom_outer.set_border_path_out(true);

        invariant::Domain<> dom_inner(&subpaving, FULL_WALL);
        dom_inner.set_border_path_in(true);
        dom_inner.set_border_path_out(true);

        // ****** Dynamics ******* //
        ibex::Function f(x1, x2, -Return(x2,
                                         (1.0*(1.0-pow(x1, 2))*x2-x1)));
        ibex::Function f2(x1, x2, -Return(x2,
                                          (1.0*(1.0-pow(x1, 2))*x2-x1)));
        DynamicsFunction dyn_outer(&f, FWD); // Duplicate because of simultaneous access of f (semaphore on DynamicsFunction)
        DynamicsFunction dyn_inner(&f2, FWD);

        // ******* Maze ********* //
        invariant::Maze<> maze_outer(&dom_outer, &dyn_outer);
        invariant::Maze<> maze_inner(&dom_inner, &dyn_inner);

        vibes::beginDrawing();
        VibesMaze v_maze("SmartSubPaving", &maze_outer, &maze_inner);

        // ******* Algorithm ********* //
        double time_start = omp_get_wtime();

        for(int i=0; i<20; i++){
            cout << i << endl;
            double time_local = omp_get_wtime();

            subpaving.bisect();
            maze_outer.contract();
            maze_inner.contract();

            // Stats
            double t = omp_get_wtime() - time_local;
            memory.push_back(array<double, 5>{i, t, v_maze.get_volume(),  v_maze.get_volume(true), subpaving.get_paves().size()});

        }
        cout << "TIME = " << omp_get_wtime() - time_start << endl;

//        v_maze.setProperties(0, 0, 1024, 1024);
//        v_maze.set_enable_cone(false);
//        v_maze.show();
    }

    save_stat_to_file("stat.txt", memory);

    //    IntervalVector position_info(2);
    //    position_info[0] = ibex::Interval(-1);
    //    position_info[1] = ibex::Interval(2.5);
    ////    v_maze.show_room_info(&maze_outer, position_info);

    //    std::string file = "/home/lemezoth/workspaceQT/tikz-adapter/tikz/figs/svg/";
    //    file += "van_der_pol_largest_negative_invariant.svg";
    //    vibes::saveImage(file);
    //    vibes::endDrawing();

}
