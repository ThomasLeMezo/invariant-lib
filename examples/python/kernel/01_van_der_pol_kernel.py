from pyinvariant import *

# Define the search space
space = IntervalVector([[-6, 6],[-6,6]])

# Create the Subpaving structure
subPaving = SmartSubPaving(space)

# Create the Domain
dom_outer = Domain(subPaving, FULL_DOOR)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)

dom_inner = Domain(subPaving, FULL_WALL)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(False)

# Create the Dynamics
f0 = Function("x[2]", "(x[1]+[-1,1],(1.0*(1.0-x[0]^2))*x[1]-x[0]+[-1,1])")
dyn_outer = DynamicsFunction(f0, FWD, False)
f1 = Function("x[2]", "(x[1],(1.0*(1.0-x[0]^2))*x[1]-x[0]-1)")
f2 = Function("x[2]", "(x[1],(1.0*(1.0-x[0]^2))*x[1]-x[0]+1)")
f3 = Function("x[2]", "(x[1]+1,(1.0*(1.0-x[0]^2))*x[1]-x[0])")
f4 = Function("x[2]", "(x[1]-1,(1.0*(1.0-x[0]^2))*x[1]-x[0])")
dyn_inner = DynamicsFunction([f1, f2, f3, f4], FWD, False)

# Create the Maze associated with the Domain and the dynamics
maze_outer = Maze(dom_outer, dyn_outer)
maze_inner = Maze(dom_inner, dyn_inner)

# Contract the system
for i in range(16): # Number of bisections
	print(i)
	subPaving.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesMaze("Van Der Pol Bracketing Invariant", maze_outer, maze_inner)
visu.setProperties(0,0,512,512)
visu.show()