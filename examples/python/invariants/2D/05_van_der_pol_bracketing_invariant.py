from pyinvariant import *

# Define the search space
space = IntervalVector([[-3, 3],[-3,3]])

# Create the Subpaving structure
subPaving = SmartSubPaving(space)

# Create the Domain
dom_outer = Domain(subPaving, FULL_DOOR)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)

dom_inner = Domain(subPaving, FULL_WALL)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(True)

# Create the Dynamics
f = Function("x[2]", "(x[1],(1.0*(1.0-x[0]^2))*x[1]-x[0])")
dyn_outer = DynamicsFunction(f, FWD_BWD)
dyn_inner = DynamicsFunction(f, FWD)

# Create the Maze associated with the Domain and the dynamics
maze_outer = Maze(dom_outer, dyn_outer)
maze_inner = Maze(dom_inner, dyn_inner)

# Contract the system
for i in range(15): # Number of bisections
	print(i)
	subPaving.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesMaze("Van Der Pol Bracketing Invariant", maze_outer, maze_inner)
visu.setProperties(0,0,512,512)
visu.show()