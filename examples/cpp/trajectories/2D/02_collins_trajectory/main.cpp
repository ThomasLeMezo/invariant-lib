#include "ibex/ibex_SepFwdBwd.h"
#include "graph.h"
#include "domain.h"
#include "dynamics_function.h"
#include "maze.h"
#include "vibes_graph.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{
    ibex::Variable x1, x2;

    IntervalVector space(2);
    space[0] = Interval(-5,5);
    space[1] = Interval(-3.5,10);
    Graph graph(space);

    // ****** Dynamics *******
    ibex::Function f(x1, x2, Return(2*x1-x1*x2,2*pow(x1,2)-x2));
    vector<Function *> f_list;
    f_list.push_back(&f);
    Dynamics_Function dyn(f_list, FWD);

    // ****** Domain & Maze *******
    invariant::Domain dom_A(&graph, FULL_WALL, LINK_TO_INITIAL_CONDITION);
    dom_A.set_border_path_in(false);
    dom_A.set_border_path_out(false);

    Function f_sep_A(x1, x2, pow(x1, 2)+pow(x2+2, 2)-1);
    SepFwdBwd s_A(f_sep_A, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_A.set_sep(&s_A);

    Maze maze_A(&dom_A, &dyn);

    invariant::Domain dom_B(&graph, FULL_WALL, LINK_TO_INITIAL_CONDITION);
    dom_B.set_border_path_in(false);
    dom_B.set_border_path_out(false);

    Function f_sep_B(x1, x2, pow(x1, 2)+pow(x2-1, 2)-pow(9.0/100.0,2));
    SepFwdBwd s_B(f_sep_B, LEQ); // LT, LEQ, EQ, GEQ, GT)
    dom_B.set_sep(&s_B);

    ibex::Function fB(x1, x2, Return(-(2*x1-x1*x2),-(2*pow(x1,2)-x2)));
    vector<Function *> f_listB;
    f_listB.push_back(&fB);
    Dynamics_Function dynB(f_listB, FWD);

    Maze maze_B(&dom_B, &dynB);

    dom_B.add_maze(&maze_A);
    dom_A.add_maze(&maze_B);

    double time_start = omp_get_wtime();
    maze_A.init(); // To set first pave to be in
    maze_B.init(); // To set first pave to be in

    int iterations = 16;
    for(int i=0; i<iterations; i++){
        cout << i << "/" << iterations-1 << endl;
        graph.bisect();
        maze_A.contract();
        maze_B.contract();

        maze_A.contract_inter(&maze_B);
        maze_B.contract_inter(&maze_A);

//        maze_A.contract();
//        maze_B.contract();
    }
    cout << "TIME = " << omp_get_wtime() - time_start << endl;

    cout << graph << endl;

    Vibes_Graph v_graphA("graph_A", &graph, &maze_A);
    v_graphA.setProperties(0, 0, 512, 512);
    v_graphA.show();
    vibes::drawCircle(0.0, -2.0, 1.0, "r[]");

    Vibes_Graph v_graphB("graph_B", &graph, &maze_B);
    v_graphB.setProperties(0, 0, 512, 512);
    v_graphB.show();
    vibes::drawCircle(0.0, 1.0, 9.0/100.0, "r[]");

    vibes::endDrawing();

//    IntervalVector position_info(2);
//    position_info[0] = Interval(-2);
//    position_info[1] = Interval(4);
//    v_graph.get_room_info(&maze, position_info);

}
