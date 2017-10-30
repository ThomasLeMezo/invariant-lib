from pyinvariant import *

# Define the search space
space = IntervalVector([[-10, 10],[-10,10]])

# Create the Subpaving structure
subPaving = SmartSubPaving(space)

# Create the Domain
dom = Domain(subPaving, FULL_DOOR)
dom.set_border_path_in(False)
dom.set_border_path_out(False)

# Create the Dynamics

# cos(tanh(2*exp(-((x[0]+2)^2+(x[1]+2)^2)/10.0) + 2*exp(-((x[0]-2)^2+(x[1]-2)^2)/10.0)-10.0+9.0)+Interval.PI/2.0)
# sin(tanh(2*exp(-((x[0]+2)^2+(x[1]+2)^2)/10.0) + 2*exp(-((x[0]-2)^2+(x[1]-2)^2)/10.0)-10.0+9.0)+Interval.PI/2.0)

def h(X1,X2):
    return 2*exp(-((X1+2)**2+(X2+2)**2)/10) + 2*exp(-((X1-2)**2+(X2-2)**2)/10) - 10

f = Function("x[2]", "(cos(atan(2.0*exp(-((x[0]+2.0)^2+(x[1]+2.0)^2)/10.0) + 2.0*exp(-((x[0]-2.0)^2+(x[1]-2.0)^2)/10.0)-10.0+9.0)+Interval.pi/2.0), \
	sin(atan(2.0*exp(-((x[0]+2.0)^2+(x[1]+2.0)^2)/10.0) + 2.0*exp(-((x[0]-2.0)^2+(x[1]-2.0)^2)/10.0)-10.0+9.0)+3.14159/2.0))")
dyn = DynamicsFunction(f, FWD_BWD)

# Create the Maze associated with the Domain and the dynamics
maze = Maze(dom, dyn)

# Contract the system
maze.init()
for i in range(0): # Number of bisections
	print(i)
	subPaving.bisect()
	maze.contract()

# Visualization
visu = VibesMaze("Cordeliere Invariant", maze)
visu.setProperties(0,0,512,512)
visu.show()