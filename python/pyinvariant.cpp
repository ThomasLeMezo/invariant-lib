#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ibex_IntervalVector.h"
#include "ibex_Function.h"
#include "ibex_Sep.h"

#include "smartSubPaving.h"
#include "domain.h"
#include "maze.h"
#include "sepmaze.h"
#include "dynamics.h"
#include "dynamics_function.h"
#include "vibesMaze.h"
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
       .value("FULL_DOOR", invariant::DOMAIN_INITIALIZATION::FULL_DOOR)
       .export_values();

   py::enum_<invariant::DYNAMICS_SENS>(m, "DYNAMICS_SENS")
       .value("FWD", invariant::DYNAMICS_SENS::FWD)
       .value("BWD", invariant::DYNAMICS_SENS::BWD)
       .value("FWD_BWD", invariant::DYNAMICS_SENS::FWD_BWD)
       .export_values();

  // ********* Graphs *********
  py::class_<invariant::SmartSubPavingIBEX>(m, "SmartSubPaving")
          .def(py::init<const IntervalVector&>(), "IntervalVector"_a)
          .def("bisect", &invariant::SmartSubPavingIBEX::bisect)
          .def("size", &invariant::SmartSubPavingIBEX::size)
  ;  

  // ********* Domain *********
  py::class_<invariant::DomainIBEX>(m, "Domain")
          .def(py::init<invariant::SmartSubPavingIBEX*, invariant::DOMAIN_INITIALIZATION>(),
               "paving"_a,
               "DOMAIN_INITIALIZATION"_a)
          .def("set_border_path_in", &invariant::DomainIBEX::set_border_path_in)
          .def("set_border_path_out", &invariant::DomainIBEX::set_border_path_out)
          .def("set_sep", &invariant::DomainIBEX::set_sep)
          .def("set_sep_input", &invariant::DomainIBEX::set_sep_input)
          .def("set_sep_output", &invariant::DomainIBEX::set_sep_output)
          .def("add_maze_union", (void (invariant::DomainIBEX::*)(std::vector<invariant::MazeIBEX *> maze_list)) &invariant::DomainIBEX::add_maze_union)
          .def("add_maze_union", (void (invariant::DomainIBEX::*)(invariant::MazeIBEX *maze)) &invariant::DomainIBEX::add_maze_union)
          .def("add_maze_inter", (void (invariant::DomainIBEX::*)(std::vector<invariant::MazeIBEX *> maze_list)) &invariant::DomainIBEX::add_maze_inter)
          .def("add_maze_inter", (void (invariant::DomainIBEX::*)(invariant::MazeIBEX *maze)) &invariant::DomainIBEX::add_maze_inter)
    ;

  // ********* Dynamics Function *********
  py::class_<invariant::Dynamics> dynamics(m, "Dynamics")
  ;

  py::class_<invariant::Dynamics_Function>(m, "DynamicsFunction", dynamics)
          .def(py::init<ibex::Function*, invariant::DYNAMICS_SENS>(),
               "f"_a,
               "DYNAMICS_SENS"_a)
          .def(py::init<std::vector<ibex::Function*>, invariant::DYNAMICS_SENS>(),
               "f_list"_a,
               "DYNAMICS_SENS"_a)
  ;

  // ********* Maze *********
  py::class_<invariant::MazeIBEX>(m, "Maze")
          .def(py::init<invariant::DomainIBEX*, invariant::Dynamics*>(),
               "domain"_a,
               "dynamics"_a)
          .def("contract", &invariant::MazeIBEX::contract)
    ;

  // SepMaze
  py::object sep = (py::object) py::module::import("pyibex").attr("Sep");
  py::class_<invariant::SepMazeIBEX>(m, "SepMaze", sep)
          .def(py::init<invariant::MazeIBEX*>(),"maze"_a)
          .def("separate", &invariant::SepMazeIBEX::separate)
  ;

  // ********* VibesMaze *********
  py::enum_<VibesMaze::VIBES_MAZE_TYPE>(m, "VIBES_MAZE_TYPE")
      .value("VIBES_MAZE_INNER", VibesMaze::VIBES_MAZE_INNER)
      .value("VIBES_MAZE_OUTER", VibesMaze::VIBES_MAZE_OUTER)
      .export_values()
  ;

  py::class_<VibesMaze>(m, "VibesMaze")
          .def(py::init<const std::string&,
               invariant::MazeIBEX*,
               VibesMaze::VIBES_MAZE_TYPE>(),
               "name"_a,
               "maze"_a,
               "VIBES_MAZE_TYPE"_a = VibesMaze::VIBES_MAZE_OUTER)
          .def(py::init<const std::string&,
               invariant::MazeIBEX*,
               invariant::MazeIBEX*>(),
               "name"_a,
               "maze_outer"_a,
               "maze_inner"_a)
          .def("setProperties", &VibesMaze::setProperties)
          .def("show", &VibesMaze::show)
          .def("drawCircle", &VibesMaze::drawCircle)
          .def("drawBox", (void (VibesMaze::*)(const ibex::IntervalVector &box, std::string params) const) &VibesMaze::drawBox)
          .def("drawBox", (void (VibesMaze::*)(double x_min, double x_max, double y_min, double y_max, std::string params) const) &VibesMaze::drawBox)
  ;

}
