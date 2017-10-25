from pyinvariant import *

# Define the search space
space = IntervalVector([[-4,4],[-3.5,6]])

# Create the grpah structure
smartSubPaving = SmartSubPaving(space)

# Create the Domain
dom_outer = Domain(smartSubPaving, FULL_WALL)
dom_outer.set_border_path_in(False)
dom_outer.set_border_path_out(False)
f_sep1 = Function("x[2]", "(x[0])^2+(x[1]+2)^2-(1)^2")
f_sep2 = Function("x[2]", "(x[0]-1)^2+(x[1]+3)^2-(0.1)^2")
s_outer1 = SepFwdBwd(f_sep1, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_outer2 = SepFwdBwd(f_sep2, LEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_outer_union = SepUnion([s_outer1, s_outer2])
dom_outer.set_sep_output(s_outer_union);

dom_inner = Domain(smartSubPaving, FULL_DOOR)
dom_inner.set_border_path_in(True)
dom_inner.set_border_path_out(True)
s_inner1 = SepFwdBwd(f_sep1, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_inner2 = SepFwdBwd(f_sep2, GEQ) # possible options : LT, LEQ, EQ, GEQ, GT
s_inner_inter = SepInter([s_inner1, s_inner2])
dom_inner.set_sep_output(s_inner_inter);

# Create the Dynamics
f_outer = Function("x[2]", "(2*x[0]-x[0]*x[1],2*x[0]^2-x[1])")
dyn_outer = DynamicsFunction(f_outer, FWD)

f_inner = Function("x[2]", "-(2*x[0]-x[0]*x[1],2*x[0]^2-x[1])")
dyn_inner = DynamicsFunction(f_inner, BWD)

# Create the two Maze associated with the Domain and the dynamics
maze_inner = Maze(dom_inner, dyn_inner)
maze_outer = Maze(dom_outer, dyn_outer)

# Contract the system
maze_outer.init()
maze_inner.init()
for i in range(15):
	print(i)
	smartSubPaving.bisect()
	maze_outer.contract()
	maze_inner.contract()

# Visualization
visu = VibesGraph("Safety Prrof Integration", smartSubPaving, maze_outer, maze_inner)
visu.setProperties(0,0,512,512)
visu.show()
visu.drawCircle(0, -2, 1, "black[red]");
visu.drawCircle(1, -3, 0.1, "black[red]");
visu.drawCircle(0, 1, 9.0/100.0, "black[green]");