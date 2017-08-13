#ifndef GRAPHIZ_GRAPH_H
#define GRAPHIZ_GRAPH_H

#include "graph.h"
#include <string>

class Graphiz_Graph
{
public:
    Graphiz_Graph(const std::string &file_name, invariant::Graph *g);
};

#endif // GRAPHIZ_GRAPH_H
