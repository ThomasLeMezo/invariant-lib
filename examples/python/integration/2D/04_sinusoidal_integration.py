from pyinvariant import *

# Define the search space
space = IntervalVector([[0,6],[-3,3]])

# Create the grpah structure
smartSubPaving = SmartSubPaving(space)

# Create the Domain
dom_outer = Domain(smartSubPaving, FULL_WALL)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)
f_sep1 = Function("x[2]", "x[0]-0.5")
f_sep2 = Function("x[2]", "x[1]^2-(2)^2")
s_outer1 = SepFwdBwd(f_sep1, LT) # possible options : LT, LEQ, EQ, GEQ, GT
s_outer2 = SepFwdBwd(f_sep2, LT) # possible options : LT, LEQ, EQ, GEQ, GT
s_outer_inter = SepInter([s_outer1, s_outer2])
dom_outer.set_sep_output(s_outer_inter);

dom_inner = Domain(smartSubPaving, FULL_DOOR)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(True)
s_inner1 = SepFwdBwd(f_sep1, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_inner2 = SepFwdBwd(f_sep2, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_inner_union = SepUnion([s_inner1, s_inner2])
dom_inner.set_sep_output(s_inner_union);

# Create the Dynamics
f = Function("x[2]", "(1,-sin(x[1]))")
dyn = DynamicsFunction(f, Dynamics::FWD)

# Create the two Maze associated with the Domain and the dynamics
maze_inner = Maze(dom_inner, dyn)
maze_outer = Maze(dom_outer, dyn)

# Contract the system
maze_outer.init()
maze_inner.init()
for i in range(15):
	print(i)
	smartSubPaving.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesMaze("Sinusoidal Intergation", maze_outer, maze_inner)
visu.setProperties(0,0,512,512)
visu.show()
visu.drawBox(0, 0.5, -2, 2, "black[red]")