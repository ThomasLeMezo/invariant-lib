from pyinvariant import *

# Define the search space
space = IntervalVector([[-3, 3],[-3,3]])

# Create the Subpaving structure
subPaving = SmartSubPaving(space)

# Create the Domain
dom = Domain(subPaving, FULL_DOOR)
dom.set_border_path_in(False)
dom.set_border_path_out(False)

# Create the Dynamics
f = Function("x[2]", "(x[1],(1.0*(1.0-x[0]^2))*x[1]-x[0])")
dyn = DynamicsFunction(f, FWD_BWD, False)

# Create the Maze associated with the Domain and the dynamics
maze = Maze(dom, dyn)

# Contract the system
for i in range(12): # Number of bisections
	print(i)
	subPaving.bisect()
	maze.contract()

# Visualization
visu = VibesMaze("Van Der Pol Invariant", maze)
visu.setProperties(0,0,512,512)
visu.show()
