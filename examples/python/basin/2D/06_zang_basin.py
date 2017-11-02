from pyinvariant import *

# Define the search space
space = IntervalVector([[-3.0, 3.0],[-3.0,3.0]])

# Create the grpah structure
subpaving = SmartSubPaving(space)

# Create Domains
f_sep1 = Function("x[2]", "(x[0]+0.852505)^2+(x[1])^2-(0.5)^2")
f_sep2 = Function("x[2]", "(x[0]-0.852505)^2+(x[1])^2-(0.5)^2")

dom_outer = Domain(subpaving, FULL_WALL)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)
s_outer1 = SepFwdBwd(f_sep1, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_outer2 = SepFwdBwd(f_sep2, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_outer = SepUnion([s_outer1, s_outer2])
dom_outer.set_sep_output(s_outer);

dom_inner = Domain(subpaving, FULL_DOOR)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(True)
s_inner1 = SepFwdBwd(f_sep1, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_inner2 = SepFwdBwd(f_sep2, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_inner = SepInter([s_inner1, s_inner2])
dom_inner.set_sep_input(s_inner);

# Create the Dynamics
f = Function("x[2]", "(x[1],8.0/25.0*x[0]^5-4.0/3.0*x[0]^3+4.0/5.0*x[0])")
dyn = DynamicsFunction(f, BWD)

# Create the two Maze associated with the Domain and the dynamics
maze_inner = Maze(dom_inner, dyn)
maze_outer = Maze(dom_outer, dyn)

# Contract the system
for i in range(20):
	print(i)
	subpaving.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesMaze("Zang basin of Capture", maze_outer, maze_inner)
visu.setProperties(0,0,1024,1024)
visu.show()
visu.drawCircle(0.852505, 0.0, 0.8, "red[]");
visu.drawCircle(-0.852505, 0.0, 0.8, "red[]");


