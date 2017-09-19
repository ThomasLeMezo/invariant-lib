from pyibex import *
from pyinvariant import *
from vibes import *

# Define the search space
space = IntervalVector([[-5.0, 5.0],[-3.5,10.0]])

# Create the grpah structure
graph = Graph(space)

# Create the Dynamics
f = Function("x[2]", "(2*x[0]-x[0]*x[1],2*x[0]^2-x[1])")
dyn = DynamicsFunction(f)

# Create the Domain for the start condition
domA = Domain(graph, LINK_TO_INITIAL_CONDITION)
domA.set_border_path_in(False)
domA.set_border_path_out(False)

f_sep_A = Function("x[2]", "x[0]^2+(x[1]+2)^2-1.0")
s_A = SepFwdBwd(f_sep_A, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT

domA.set_sep(s_A);
mazeA = Maze(domA, dyn, MAZE_FWD, MAZE_WALL)

# Create the Domain for the ending condition
domB = Domain(graph, LINK_TO_INITIAL_CONDITION)
domB.set_border_path_in(False)
domB.set_border_path_out(False)

f_sep_B = Function("x[2]", "x[0]^2+(x[1]-1)^2-9.0/100.0")
s_B = SepFwdBwd(f_sep_B, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT

domB.set_sep(s_B);
mazeB = Maze(domB, dyn, MAZE_FWD, MAZE_DOOR)

domB.add_maze(mazeA)
domA.add_maze(mazeB)

# Contract the system
mazeA.init()
mazeB.init()
for i in range(9):
	print(i)
	graph.bisect()

	# Contract maze independently
	mazeA.contract()
	mazeB.contract()

	# Contract inter maze
	mazeA.contract_inter(mazeB)
	mazeB.contract_inter(mazeA)

	# Visualization
	visuA = VibesGraph("graphA", graph, mazeA)
	visuA.setProperties(0,0,512,512)
	visuA.show()
	visuA.drawCircle(0.0, -2.0, 1.0, "r[]") # (x_center, y_center, radius, color)

	visuB = VibesGraph("graphB", graph, mazeB)
	visuB.setProperties(512,0,512,512)
	visuB.show()
	visuB.drawCircle(0.0, 1.0, 9.0/100.0, "r[]") # (x_center, y_center, radius, color)

	input()


