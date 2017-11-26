from pyinvariant import *

# Define the search space
space = IntervalVector([[-3.0, 3.0],[-3.0,3.0]])

# Create the grpah structure
subpaving = SmartSubPaving(space)

# Create Domains
dom_outer = Domain(subpaving, FULL_WALL)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)
f_sep = Function("x[2]", "(x[0])^2+(x[1])^2-(0.4)^2")
s_outer = SepFwdBwd(f_sep, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT
dom_outer.set_sep_output(s_outer);

dom_inner = Domain(subpaving, FULL_DOOR)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(True)
s_inner = SepFwdBwd(f_sep, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
dom_inner.set_sep_input(s_inner);

# Create the Dynamics
f = Function("x[2]", "-(x[1],(1.0-(x[0]^2))*x[1]-x[0])") # Repulsive cycle
dyn = DynamicsFunction(f, Dynamics::BWD)

# Create the two Maze associated with the Domain and the dynamics
maze_inner = Maze(dom_inner, dyn)
maze_outer = Maze(dom_outer, dyn)

# Contract the system
for i in range(13):
	print(i)
	subpaving.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesMaze("Van Der Pol Basin of Capture", maze_outer, maze_inner)
visu.setProperties(0,0,512,512)
visu.show()
visu.drawCircle(0.0, 0.0, 0.4, "black[red]");


