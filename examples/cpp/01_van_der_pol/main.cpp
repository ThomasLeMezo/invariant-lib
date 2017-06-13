#include "graph.h"
#include "pave.h"
#include "VibesFigure_Graph.h"
#include <iostream>
#include "vibes/vibes.h"
#include <cstring>

using namespace std;
using namespace ibex;
using namespace invariant;

int main(int argc, char *argv[])
{

    cout << "HELLO" << endl;
    IntervalVector space(2);
    space[0] = Interval(0,1);
    space[1] = Interval(0,1);

    Graph g(space);
//    cout << g << endl;
    for(int i=0; i<3; i++){
        g.bisect();
        VibesFigure_Graph visu_g("graph " + to_string(i), &g);
        visu_g.show();
        for(Pave* p:g.paves()){
            for(Face* f:p->getFaces_vector()){
                if(f->coordinates().size()==0)
                    cout << "error dim" << endl;
            }
        }
    }
    cout << g << endl;
}
