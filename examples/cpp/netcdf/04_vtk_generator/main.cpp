#include <iostream>
#include "vtk_graph.h"

using namespace std;
using namespace invariant;
using namespace ibex;

int main(int argc, char *argv[])
{
    Vtk_Graph vtk_graph("Previmer");
    vtk_graph.show_maze("current.maze");
    return 0;
}
