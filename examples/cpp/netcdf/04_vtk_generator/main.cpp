#include <iostream>
#include "vtkMaze3D.h"

using namespace std;
using namespace invariant;
using namespace ibex;

int main(int argc, char *argv[])
{
    VtkMaze3D vtkMaze3D("Previmer");
    vtkMaze3D.show_maze("current.maze");
    return 0;
}
