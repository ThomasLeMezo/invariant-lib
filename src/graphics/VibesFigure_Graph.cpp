#include "VibesFigure_Graph.h"
#include <ibex.h>
#include "vibes.h"

using namespace invariant;
using namespace ibex;
using namespace std;

VibesFigure_Graph::VibesFigure_Graph(const std::string& figure_name, Graph *g): VibesFigure(figure_name){
    m_graph = g;
}

void VibesFigure_Graph::show() const{
    vibes::newGroup("graph_bisectable", "gray[gray]", vibesParams("figure", m_name));
    vibes::newGroup("graph_not_bisectable", "lightGray[lightGray]", vibesParams("figure", m_name));


    vibes::Params params_bisectable, params_not_bisectable;
    params_bisectable = vibesParams("figure", m_name, "group", "graph_bisectable", "FaceColor","none","EdgeColor","gray");
    params_not_bisectable = vibesParams("figure", m_name, "group", "graph_not_bisectable", "FaceColor","none","EdgeColor","lightGray");

    for(Pave*p:m_graph->paves()){
        ibex::IntervalVector box(p->coordinates());
        vibes::drawBox(box, params_bisectable);
    }
    for(Pave*p:m_graph->paves_not_bisectable())
        vibes::drawBox(p->coordinates(), params_not_bisectable);

    vibes::axisAuto(m_name);
//    vibes::axisLimits(0, 0, 0, 0);
}

