#include "smartSubPaving.h"
#include "pave.h"
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
    IntervalVector space(2);
    space[0] = ibex::Interval(0,1);
    space[1] = ibex::Interval(0,1);

    SmartSubPavingIBEX paving(space);
    double t_start = omp_get_wtime();
    for(int i=0; i<4; i++){
//        paving.bisect_monothread();
        paving.bisect();
    }
    cout << omp_get_wtime() - t_start << endl;

    vibes::beginDrawing();
    VibesMaze visu_g("graph", &paving);
    visu_g.setProperties(0, 0, 512, 512);
    visu_g.show();
    vibes::endDrawing();

    Graphiz_Graph ggraph("graph.pdf", &paving);
    cout << paving << endl;
}
