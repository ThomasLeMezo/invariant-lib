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
    invariant::Graph*  m_graph;
    double             m_overhead_factor;

};

namespace vibes{
    VIBES_FUNC_COLOR_PARAM_1(drawGraph,const invariant::Graph &, g);
}

#endif // Vibes_Graph_H
