#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/functional.h>

#include "graph.h"
#include "domain.h"
#include "maze.h"
#include "dynamics_function.h"
#include "vibes_graph.h"

namespace py = pybind11;
using namespace pybind11::literals;
using py::self;

PYBIND11_MODULE(pyinvariant, m){
  m.doc() = "Python binding of invariant-lib";

//  py::object pyibex_function = (py::object) py::module::import("pyibex").attr("Function");
//  py::object pyibex_iv = (py::object) py::module::import("pyibex").attr("IntervalVector");

  // Graphs
  py::class_<invariant::Graph>(m, "Graph")
          .def(py::init<const ibex::IntervalVector&>(), "space"_a )
  ;

  // Domain
  py::class_<invariant::Domain>(m, "Domain")
          ;

  // Dynamics Function
  py::class_<invariant::Dynamics_Function>(m, "DynamicsFunction")
          ;

  // Maze
  py::class_<invariant::Maze>(m, "Maze")
          ;

  // Vibes_Graph
  py::class_<Vibes_Graph>(m, "VibesGraph")
          ;
}
