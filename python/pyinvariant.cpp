#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ibex/ibex_IntervalVector.h"
#include "ibex/ibex_Function.h"
#include "ibex/ibex_Sep.h"

#include "smartSubPaving.h"
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
   py::enum_<invariant::DOMAIN_INITIALIZATION>(m, "DOMAIN_INITIALIZATION")
       .value("FULL_WALL", invariant::DOMAIN_INITIALIZATION::FULL_WALL)
       .value("FULL_DOOR", invariant::DOMAIN_INITIALIZATION::FULL_WALL)
       .export_values();

   py::enum_<invariant::DYNAMICS_SENS>(m, "DYNAMICS_SENS")
       .value("FWD", invariant::DYNAMICS_SENS::FWD)
       .value("BWD", invariant::DYNAMICS_SENS::BWD)
       .value("FWD_BWD", invariant::DYNAMICS_SENS::FWD_BWD)
       .export_values();

   py::enum_<DOMAIN_PROPAGATION_START>(m, "DOMAIN_PROPAGATION_START")
       .value("LINK_TO_INITIAL_CONDITION", DOMAIN_PROPAGATION_START::LINK_TO_INITIAL_CONDITION)
       .value("NOT_LINK_TO_INITIAL_CONDITION", DOMAIN_PROPAGATION_START::NOT_LINK_TO_INITIAL_CONDITION)
       .export_values();

  // ********* Graphs *********
  py::class_<SmartSubPaving>(m, "SmartSubPaving")
          .def(py::init<const IntervalVector&>(), "IntervalVector"_a)
          .def("bisect", &SmartSubPaving::bisect)
          .def("size", &SmartSubPaving::size)
  ;  

  // ********* Domain *********
  py::class_<invariant::Domain>(m, "Domain")
          .def(py::init<invariant::SmartSubPaving*, invariant::DOMAIN_INITIALIZATION, invariant::DOMAIN_PROPAGATION_START>(),
               "paving"_a,
               "DOMAIN_INITIALIZATION"_a,
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
  py::class_<invariant::Dynamics> dynamics(m, "Dynamics")
  ;

  py::class_<invariant::Dynamics_Function>(m, "DynamicsFunction", dynamics)
          .def(py::init<ibex::Function*, invariant::DYNAMICS_SENS>(),
               "f"_a,
               "DYNAMICS_SENS"_a)
  ;

  // ********* Maze *********
  py::class_<invariant::Maze>(m, "Maze")
          .def(py::init<invariant::Domain*, invariant::Dynamics*>(),
               "domain"_a,
               "dynamics"_a)
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
               invariant::SmartSubPaving*,
               invariant::Maze*,
               Vibes_Graph::VIBES_GRAPH_TYPE>(),
               "name"_a,
               "paving"_a,
               "maze"_a,
               "VIBES_GRAPH_TYPE"_a = Vibes_Graph::VIBES_GRAPH_OUTER)
          .def(py::init<const std::string&,
               invariant::SmartSubPaving*,
               invariant::Maze*,
               invariant::Maze*>(),
               "name"_a,
               "paving"_a,
               "maze_outer"_a,
               "maze_inner"_a)
          .def("setProperties", &Vibes_Graph::setProperties)
          .def("show", &Vibes_Graph::show)
          .def("drawCircle", &Vibes_Graph::drawCircle)
  ;

}
