#ifndef Vibes_Graph_H
#define Vibes_Graph_H

#include "ibex.h"
#include "graph.h"
#include "VibesFigure.h"
#include "vibes/vibes.h"

class Vibes_Graph: public VibesFigure
{
public:
    Vibes_Graph(const std::string& figure_name, invariant::Graph *g);
    ~Vibes_Graph(){}

    virtual void show() const;

private:
    invariant::Graph*  m_graph = NULL;
    double             m_overhead_factor;

};

namespace vibes{
    VIBES_FUNC_COLOR_PARAM_1(drawGraph,const invariant::Graph &, g)
    VIBES_FUNC_COLOR_PARAM_1(drawPave,const invariant::Pave &, p)
    VIBES_FUNC_COLOR_PARAM_1(drawFace,const invariant::Face &, f)
    VIBES_FUNC_COLOR_PARAM_1(drawPave,const std::vector<invariant::Pave*> &, l_p)
    VIBES_FUNC_COLOR_PARAM_1(drawFace,const std::vector<invariant::Face*> &, l_f)
}

#endif // Vibes_Graph_H
