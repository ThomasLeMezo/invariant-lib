from pyibex import *
from pyinvariant import *

# Define the search space
space = IntervalVector([[-3, 3],[-3,3]])

# Create the grpah structure
graph = Graph(space)

# Create the Domain
dom = Domain(graph)

dom.set_border_path_in(False)
dom.set_border_path_out(False)

# Create the Dynamics
f = Function("x[2]", "(x[1],(1.0*(1.0-x[0]^2))*x[1]-x[0])")
dyn = DynamicsFunction(f)

# Create the Maze associated with the Domain and the dynamics
maze = Maze(dom, dyn, MAZE_FWD_BWD, MAZE_DOOR)

# Contract the system
for i in range(12):
	print(i)
	graph.bisect()
	maze.contract()

# Visualization
visu = VibesGraph("graph", graph, maze)
visu.setProperties(0,0,512,512)
visu.show()


