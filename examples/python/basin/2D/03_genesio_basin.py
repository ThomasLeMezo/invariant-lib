from pyibex import *
from pyinvariant import *

# Define the search space
space = IntervalVector([[-70, 70],[-200,200]])

# Create the grpah structure
graph = Graph(space)

# Create Domains
# - Outer
dom_outer = Domain(graph)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)

f_sep_outer = Function("x[2]", "(x[0])^2+(x[1])^2-(3.0)^2")
s_outer = SepFwdBwd(f_sep_outer, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT
dom_outer.set_sep(s_outer);

# - Innter
dom_inner = Domain(graph)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(True)

f_sep_inner = Function("x[2]", "(x[0])^2+(x[1])^2-(3.0)^2")
s_inner = SepFwdBwd(f_sep_inner, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
dom_inner.set_sep(s_inner);


# Create the Dynamics
f_outer = Function("x[2]", "(-(-x[0]+x[1]), -(0.1*x[0]-2*x[1]-x[0]^2-0.1*x[0]^3))")
dyn_outer = DynamicsFunction(f_outer)

f_inner = Function("x[2]", "((-x[0]+x[1]), (0.1*x[0]-2*x[1]-x[0]^2-0.1*x[0]^3))")
dyn_inner = DynamicsFunction(f_inner)

# Create the Maze associated with the Domain and the dynamics
maze_outer = Maze(dom_outer, dyn_outer, MAZE_FWD, MAZE_PROPAGATOR)
maze_inner = Maze(dom_inner, dyn_inner, MAZE_BWD, MAZE_CONTRACTOR)

# Contract the system
maze_outer.init()
for i in range(10):
	print(i)
	graph.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesGraph("graph", graph, maze_outer, maze_inner)
visu.setProperties(0,0,512,512)
visu.show()

visu.drawCircle(0.0, 0.0, 0.1, "red[]");


