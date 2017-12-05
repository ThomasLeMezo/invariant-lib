from pyinvariant import *

# Define the search space
space = IntervalVector([[-3.0, 3.0],[-3.0,3.0]])

# Create the grpah structure
subpaving = SmartSubPaving(space)

# Create Domains
f_sep1 = Function("x[2]", "(x[0]-1.0)^2-(0.4)^2")
f_sep2 = Function("x[2]", "(x[1]-0.0)^2-(0.4)^2")

f2_sep1 = Function("x[2]", "(x[0]+0.8)^2-(0.2)^2")
f2_sep2 = Function("x[2]", "(x[1]-0.5)^2-(0.2)^2")

dom_outer = Domain(subpaving, FULL_WALL)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)

## Build domain separator for outer
s1_outer1 = SepFwdBwd(f_sep1, LEQ)
s1_outer2 = SepFwdBwd(f_sep2, LEQ)
s1_outer = SepInter([s1_outer1, s1_outer2])
s2_outer1 = SepFwdBwd(f2_sep1, LEQ)
s2_outer2 = SepFwdBwd(f2_sep2, LEQ)
s2_outer = SepInter([s2_outer1, s2_outer2])
s_outer = SepUnion([s1_outer, s2_outer])
dom_outer.set_sep_output(s_outer)

dom_inner = Domain(subpaving, FULL_DOOR)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(True)

## Build domain separator for input
s1_inner1 = SepFwdBwd(f_sep1, GEQ)
s1_inner2 = SepFwdBwd(f_sep2, GEQ)
s1_inner = SepUnion([s1_inner1, s1_inner2])
s2_inner1 = SepFwdBwd(f2_sep1, GEQ)
s2_inner2 = SepFwdBwd(f2_sep2, GEQ)
s2_inner = SepUnion([s2_inner1, s2_inner2])
s_inner = SepInter([s1_inner, s2_inner])
dom_inner.set_sep_input(s_inner)

# Create the Dynamics
f = Function("x[2]", "-(x[1],(1.0-(x[0]^2))*x[1]-x[0])") # Repulsive cycle
dyn = DynamicsFunction(f, BWD)

# Create the two Maze associated with the Domain and the dynamics
maze_inner = Maze(dom_inner, dyn)
maze_outer = Maze(dom_outer, dyn)

# Contract the system
for i in range(15):
	print(i)
	subpaving.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesMaze("Van Der Pol Basin of Capture", maze_outer, maze_inner)
visu.setProperties(0,0,512,512)
visu.show()

x1_c = -0.8;
x2_c = 0.5;
r = 0.2;
visu.drawBox(x1_c-r, x1_c+r, x2_c-r, x2_c+r, "black[red]");

x1_c2 = 1.0;
x2_c2 = 0.0;
r2 = 0.4;
visu.drawBox(x1_c2-r2, x1_c2+r2, x2_c2-r2, x2_c2+r2, "black[red]");


