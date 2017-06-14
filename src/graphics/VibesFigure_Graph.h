#ifndef VIBESFIGURE_GRAPH_H
#define VIBESFIGURE_GRAPH_H

#include "ibex.h"
#include "graph.h"
#include "VibesFigure.h"

class VibesFigure_Graph: public VibesFigure
{
public:
    VibesFigure_Graph(const std::string& figure_name, invariant::Graph *g);
    ~VibesFigure_Graph(){}

    virtual void show() const;

private:
    invariant::Graph*  m_graph;

};

#endif // VIBESFIGURE_GRAPH_H
