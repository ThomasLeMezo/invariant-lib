#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ibex/ibex_IntervalVector.h"
#include "ibex/ibex_Function.h"
#include "ibex/ibex_Sep.h"

#include "graph.h"
#include "domain.h"
#include "maze.h"
#include "sepmaze.h"
#include "dynamics.h"
#include "dynamics_function.h"
#include "vibes_graph.h"
#include <string>

namespace py = pybind11;
using namespace pybind11::literals;

using namespace invariant;
using namespace ibex;

 PYBIND11_MODULE(pyinvariant, m){
   m.doc() = "Python binding of invariant-lib";

   // ********* ENUM *********
   py::enum_<invariant::MazeSens>(m, "MazeSens")
       .value("MAZE_FWD", invariant::MazeSens::MAZE_FWD)
       .value("MAZE_BWD", invariant::MazeSens::MAZE_BWD)
       .value("MAZE_FWD_BWD", invariant::MazeSens::MAZE_FWD_BWD)
       .export_values();

   py::enum_<MazeType>(m, "MazeType")
       .value("MAZE_CONTRACTOR", MazeType::MAZE_CONTRACTOR)
       .value("MAZE_PROPAGATOR", MazeType::MAZE_PROPAGATOR)
       .export_values();

   py::enum_<DOMAIN_PROPAGATION_START>(m, "DOMAIN_PROPAGATION_START")
       .value("LINK_TO_INITIAL_CONDITION", DOMAIN_PROPAGATION_START::LINK_TO_INITIAL_CONDITION)
       .value("NOT_LINK_TO_INITIAL_CONDITION", DOMAIN_PROPAGATION_START::NOT_LINK_TO_INITIAL_CONDITION)
       .export_values();

  // ********* Graphs *********
  py::class_<Graph>(m, "Graph")
          .def(py::init<const IntervalVector&>(), "IntervalVector"_a)
          .def("bisect", &Graph::bisect)
          .def("size", &Graph::size)
  ;  

  // ********* Domain *********
  py::class_<invariant::Domain>(m, "Domain")
          .def(py::init<invariant::Graph*>(), "graph"_a)
          .def(py::init<invariant::Graph*, DOMAIN_PROPAGATION_START>(),
               "graph"_a,
               "DOMAIN_PROPAGATION_START"_a = invariant::NOT_LINK_TO_INITIAL_CONDITION)
          .def("set_border_path_in", &invariant::Domain::set_border_path_in)
          .def("set_border_path_out", &invariant::Domain::set_border_path_out)
          .def("set_sep", &invariant::Domain::set_sep)
          .def("set_sep_input", &invariant::Domain::set_sep_input)
          .def("set_sep_output", &invariant::Domain::set_sep_output)
          .def("add_maze", &invariant::Domain::add_maze)
//          .def("add_remove_maze", &invariant::Domain::add_remove_maze)
//          .def("add_remove_mazes", &invariant::Domain::add_remove_mazes)
//          .def("add_remove_mazes_input", &invariant::Domain::add_remove_mazes_input)
//          .def("add_remove_mazes_output", &invariant::Domain::add_remove_mazes_output)
    ;

  // ********* Dynamics Function *********
  py::class_<invariant::Dynamics> dynamics(m, "Dynamics");
//          dynamics.def(py::init<>())
//    ;
  py::class_<invariant::Dynamics_Function>(m, "DynamicsFunction", dynamics)
          .def(py::init<ibex::Function*>(), "f"_a)
  ;

  // ********* Maze *********
  py::class_<invariant::Maze>(m, "Maze")
          .def(py::init<invariant::Domain*, invariant::Dynamics*, invariant::MazeSens, invariant::MazeType>(),
               "domain"_a,
               "dynamics"_a,
               "MazeSens"_a = invariant::MAZE_FWD_BWD,
               "MazeType"_a = invariant::MAZE_CONTRACTOR)
          .def("contract", &invariant::Maze::contract)
          .def("contract_inter", &invariant::Maze::contract_inter)
          .def("init", &invariant::Maze::init)
    ;

  // SepMaze
  py::object sep = (py::object) py::module::import("pyibex").attr("Sep");
  py::class_<invariant::SepMaze>(m, "SepMaze", sep)
          .def(py::init<invariant::Maze*>(),"maze"_a)
          .def("separate", &invariant::SepMaze::separate)
  ;

  // ********* Vibes_Graph *********
  py::enum_<Vibes_Graph::VIBES_GRAPH_TYPE>(m, "VIBES_GRAPH_TYPE")
      .value("VIBES_GRAPH_INNER", Vibes_Graph::VIBES_GRAPH_INNER)
      .value("VIBES_GRAPH_OUTER", Vibes_Graph::VIBES_GRAPH_OUTER)
      .export_values()
  ;

  py::class_<Vibes_Graph>(m, "VibesGraph")
          .def(py::init<const std::string&,
               invariant::Graph*,
               invariant::Maze*,
               Vibes_Graph::VIBES_GRAPH_TYPE>(),
               "name"_a,
               "graph"_a,
               "maze"_a,
               "VIBES_GRAPH_TYPE"_a = Vibes_Graph::VIBES_GRAPH_OUTER)
          .def(py::init<const std::string&,
               invariant::Graph*,
               invariant::Maze*,
               invariant::Maze*>(),
               "name"_a,
               "graph"_a,
               "maze_outer"_a,
               "maze_inner"_a)
          .def("setProperties", &Vibes_Graph::setProperties)
          .def("show", &Vibes_Graph::show)
          .def("drawCircle", &Vibes_Graph::drawCircle)
  ;

}
