from pyibex import *
from pyinvariant import *

# Define the search space
space = IntervalVector([[-4, 4],[-4,4]])

# Create the grpah structure
graph = Graph(space)

# Create the Domain
dom = Domain(graph)

dom.set_border_path_in(True)
dom.set_border_path_out(False)

f_sep = Function("x[2]", "(x[0])^2+(x[1])^2-(0.3)^2")

s = SepFwdBwd(f_sep, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
dom.set_sep(s);

# Create the Dynamics
f = Function("x[2]", "(x[1],(-1.0*(x[0]+x[1]^3-x[1])))")
dyn = DynamicsFunction(f)

# Create the Maze associated with the Domain and the dynamics
maze = Maze(dom, dyn, MAZE_BWD, MAZE_DOOR)

# Contract the system
for i in range(15):
	print(i)
	graph.bisect()
	maze.contract()

# Visualization
visu = VibesGraph("graph", graph, maze)
visu.setProperties(0,0,512,512)
visu.show()


