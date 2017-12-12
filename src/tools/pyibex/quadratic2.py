from pyibex import *
from vibes import *
import numpy as np
import matplotlib.pyplot as plt

######################## Functions ########################

def drawBox(x, color=''):
	vibes.drawBox(x[0].lb(), x[0].ub(), x[1].lb(), x[1].ub(), color)

def f_numpy(x1, x2):
	return [1, -np.sin(x2)]

def f_numpy2(x1, x2):
	return [0, np.cos(x2)*np.sin(x2)]

def f_n_numpy(x1, x2):
	normalization = 1000.0
	[fx1, fx2] = f_numpy(x1, x2)
	coeff = np.sqrt(fx1**2+fx2**2)*normalization
	return [fx1/coeff, fx2/coeff]

def f_n_numpy2(x1, x2):
	normalization = 1000.0
	[fx1, fx2] = f_numpy2(x1, x2)
	coeff = np.sqrt(fx1**2+fx2**2)*normalization
	return [fx1/coeff, fx2/coeff]

def draw_vector_field():
	for x in np.arange(pave[0].lb(), pave[0].ub(), pave[0].diam()/10):
		for y in np.arange(pave[1].lb(), pave[1].ub(), pave[1].diam()/10):
			vibes.drawArrow([x, y], [x+f_n_numpy(x, y)[0], y+f_n_numpy(x, y)[1]], pave[0].diam()/100, 'black[black]')
			# vibes.drawArrow([x, y], [x+f_n_numpy2(x, y)[0], y+f_n_numpy2(x, y)[1]], 0.01, 'grey[grey]')

			# vibes.drawArrow([x, y], [x+f(x, y)[0].mid(), y+f(x, y)[1].mid()], 0.01, 'black[black]')

######################## Main ########################
# pave = IntervalVector([[0, 1], [-0.5, 0.5]])
pave = IntervalVector(([1.0078125, 1.03125] , [-1.61796874999999, -1.5703125]))

# f = Function("x[2]", "(1, -sin(x[1]))")
f = Function("x[2]", "(1, [1, 2])")

# IN = IntervalVector([Interval(pave[0].lb()), [-0.25, 0.5]])
# IN = IntervalVector([[0.25, 0.5], Interval(pave[1].lb())])
# IN = IntervalVector([[0.75, 1], Interval(pave[1].lb())])
# IN = IntervalVector([[0.0, 0.25], Interval(pave[1].lb())])
IN = IntervalVector([[1.00782, 1.00782],[-1.57389, -1.57031]])

# OUT = IntervalVector([Interval(pave[0].ub()), [-0.5, 0.5]])
# OUT = IntervalVector([[0, 1], Interval(pave[1].ub())])
OUT = IntervalVector([[1.00782, 1.03125],[-1.57031, -1.57031]])

OUT_contract = IntervalVector(OUT)


######################## Compute ########################

f_d = f.diff()
vect = f.eval_vector(pave)
matrix = f_d.eval_matrix(pave)
vect_matrix = IntervalVector([matrix[0][0], matrix[1][1]])
vect_d = hadamard_product(vect_matrix, f.eval_vector(pave))
print("vect_d = ", vect_d)

print("IN = ", IN)
print("OUT = ", OUT)

out_degenerated = 0
if(OUT[1].is_degenerated()):
	out_degenerated = 1

in_degenerated = 0
if(IN[1].is_degenerated()):
	in_degenerated = 1

print("out_degenerated = ", out_degenerated)
print("in_degenerated = ", in_degenerated)

y = Interval.EMPTY_SET

in_lb = IntervalVector(IN.lb())
vect_in_lb = f.eval_vector(in_lb)
a1=0.5*vect_d[out_degenerated] 
b1=vect_in_lb[out_degenerated]
c1=in_lb[out_degenerated]-OUT[out_degenerated].lb()
	
print("----")
print("a1 = ", a1)
print("b1 = ", b1)
print("c1 = ", c1)

if(a1.lb() != 0):
	t = ((-b1-sqrt((pow(b1, 2)-4*a1.lb()*c1)))/(2*a1.lb())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_lb[1-out_degenerated]*t+in_lb[1-out_degenerated]
	t = ((-b1+sqrt((pow(b1, 2)-4*a1.lb()*c1)))/(2*a1.lb())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_lb[1-out_degenerated]*t+in_lb[1-out_degenerated]
else:
	t= -c1/b1 & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_lb[1-out_degenerated]*t+in_lb[1-out_degenerated]

if(a1.ub() != 0):
	t = ((-b1-sqrt((pow(b1, 2)-4*a1.ub()*c1)))/(2*a1.ub())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_lb[1-out_degenerated]*t+in_lb[1-out_degenerated]
	t = ((-b1+sqrt((pow(b1, 2)-4*a1.ub()*c1)))/(2*a1.ub())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_lb[1-out_degenerated]*t+in_lb[1-out_degenerated]
else:
	t= -c1/b1 & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_lb[1-out_degenerated]*t+in_lb[1-out_degenerated]

print("y=", y)

in_ub = IntervalVector(IN.ub())
vect_in_ub = f.eval_vector(in_ub)
a1=0.5*vect_d[out_degenerated]
b1=vect_in_ub[out_degenerated]
c1=in_ub[out_degenerated]-OUT[out_degenerated].lb()

print("----")
print("a1 = ", a1)
print("b1 = ", b1)
print("c1 = ", c1)

if(a1.lb() != 0):
	t = ((-b1-sqrt((pow(b1, 2)-4*a1.lb()*c1)))/(2*a1.lb())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_ub[1-out_degenerated]*t+in_ub[1-out_degenerated]
	t = ((-b1+sqrt((pow(b1, 2)-4*a1.lb()*c1)))/(2*a1.lb())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_ub[1-out_degenerated]*t+in_ub[1-out_degenerated]
else:
	t= -c1/b1 & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_ub[1-out_degenerated]*t+in_ub[1-out_degenerated]

if(a1.ub() != 0):
	t = ((-b1-sqrt((pow(b1, 2)-4*a1.ub()*c1)))/(2*a1.ub())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_ub[1-out_degenerated]*t+in_ub[1-out_degenerated]
	t = ((-b1+sqrt((pow(b1, 2)-4*a1.ub()*c1)))/(2*a1.ub())) & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_ub[1-out_degenerated]*t+in_ub[1-out_degenerated]
else:
	t= -c1/b1 & Interval.POS_REALS
	y |= 0.5*vect_d[1-out_degenerated]*(t**2)+vect_in_ub[1-out_degenerated]*t+in_ub[1-out_degenerated]

OUT_contract[1-out_degenerated] &= y
print("OUT_contract = ", OUT_contract)

######################## Drawing ########################
vibes.beginDrawing()
vibes.newFigure("Test")
vibes.setFigureSize(512, 512)
vibes.axisLimits(pave[0].lb(), pave[0].ub(), pave[1].lb(), pave[1].ub())

########## Trajectories ##########
pt_lb_list_ub = []
pt_lb_list_lb = []
pt_ub_list_ub = []
pt_ub_list_lb = []

time = np.linspace(0, 1, 100)
for t in time:
	pt_x = 0.5*vect_d[0]*(t**2)+vect_in_lb[0]*t+in_lb[0]
	pt_y = 0.5*vect_d[1]*(t**2)+vect_in_lb[1]*t+in_lb[1]
	pt_lb_list_ub.append([pt_x.ub(), pt_y.ub()])
	pt_lb_list_lb.append([pt_x.lb(), pt_y.lb()])

	pt_x = 0.5*vect_d[0]*(t**2)+vect_in_ub[0]*t+in_ub[0]
	pt_y = 0.5*vect_d[1]*(t**2)+vect_in_ub[1]*t+in_ub[1]
	pt_ub_list_ub.append([pt_x.ub(), pt_y.ub()])
	pt_ub_list_lb.append([pt_x.lb(), pt_y.lb()])

vibes.drawLine(pt_lb_list_ub, "orange")
vibes.drawLine(pt_lb_list_lb, "orange")
vibes.drawLine(pt_ub_list_ub, "orange")
vibes.drawLine(pt_ub_list_lb, "orange")

########## Boxes ##########
drawBox(pave, "black[]")

## IN
in_draw = IN
in_draw[in_degenerated] += Interval(-1, 1) * 0.01*pave[in_degenerated].diam()
drawBox(in_draw, "r[r]")

## OUT Contract
out_contract_draw = OUT_contract
out_contract_draw[out_degenerated] += Interval(-1, 1) * 0.01*pave[out_degenerated].diam()
drawBox(out_contract_draw, "b[b]")

## OUT
out_draw = OUT
out_draw[out_degenerated] += Interval(-1, 1) * 0.01*pave[out_degenerated].diam()
drawBox(out_draw, "g[]")

########## Vector Field ##########

draw_vector_field()

