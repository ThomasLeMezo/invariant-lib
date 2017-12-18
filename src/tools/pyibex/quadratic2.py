from pyibex import *
from vibes import *
import numpy as np

######################## Main ########################

# f = Function("x[2]", "(1, -sin(x[1]))")
# f = Function("x[2]", "(1, [1, 2])")

####

# pave = IntervalVector([[1, 1.5] , [1, 1.5]])
# OUT = IntervalVector([[1, 1], [1, 1.5]])
# IN = IntervalVector([[1, 1.1], [1, 1]])
# f = Function("x[2]", "-(x[1], (1-x[0]^2)*x[1]-x[0])")

# pave = IntervalVector([[3, 6], [0.050000000000000044, 3.1]])
# IN = IntervalVector([[3, 3], [0.050000000000000044, 3.1]])
# OUT = IntervalVector([[3, 6], [0.050000000000000044, 0.050000000000000044]])
# f = Function("x[2]", "(x[1], (1-x[0]^2)*x[1]-x[0])")

pave = IntervalVector([[1.875, 2.25] , [0, 0.375]])
IN = IntervalVector([[1.98588, 2.25], [0, 0]])
OUT = IntervalVector([[1.875, 2.25], [0.375, 0.375]])
f = Function("x[2]", "-(x[1], (1-x[0]^2)*x[1]-x[0])")

# pave = IntervalVector([[0, 1.5], [1.5, 3]])
# OUT = IntervalVector([[0, 1.5], [1.5, 1.5]])
# IN = IntervalVector([[1.5, 1.5], [1.5, 3]])
# f = Function("x[2]", "-(x[1], (1-x[0]^2)*x[1]-x[0])")

# pave = IntervalVector([[1.5, 3], [0, 1.5]])
# IN = IntervalVector([[1.5, 3], [0, 0]])
# OUT = IntervalVector([[1.5, 1.5], [0, 1.5]])
# f = Function("x[2]", "-(x[1], (1-x[0]^2)*x[1]-x[0])")

# pave = IntervalVector([[1.5, 3], [0, 1.5]])
# IN = IntervalVector([[1.5, 3], [0, 0]])
# OUT = IntervalVector([[1.5, 3], [1.5, 1.5]])
# f = Function("x[2]", "-(x[1], (1-x[0]^2)*x[1]-x[0])")


######################## Compute ########################
OUT_contract = IntervalVector(OUT)

f_d = f.diff()
print("f_d = ", f_d)
vect = f.eval_vector(pave)
matrix = f_d.eval_matrix(pave)
vect_d = IntervalVector(2)

# print("matrix = ", matrix)

vect_d[0] = matrix[0][0]*vect[0] + 0.5*matrix[0][1]*vect[1]
vect_d[1] = matrix[1][1]*vect[1] + 0.5*matrix[1][0]*vect[0]

print("vect = ", vect)
print("vect_d = ", vect_d)

print("IN = ", IN)
print("OUT = ", OUT)

out_degenerated = 0
if(OUT[1].is_degenerated()):
	out_degenerated = 1

in_degenerated = 0
if(IN[1].is_degenerated()):
	in_degenerated = 1

##### Test #####

ddx = vect_d
dx = f.eval_vector(IN)
x = IN
y = OUT

g = Function("ddx[2]", "dx[2]", "c[2]", "t", "(ddx[0]*t^2+dx[0]*t+c[0], ddx[1]*t^2+dx[1]*t+c[1])")
ctc = CtcFwdBwd(g)
c1 = IntervalVector(7)
c1[0] = ddx[0] & Interval.NEG_REALS
c1[1] = ddx[1] & Interval.NEG_REALS
c1[2] = dx[0] & Interval.NEG_REALS
c1[3] = dx[1] & Interval.NEG_REALS
c1[4] = x[0] - y[0]
c1[5] = x[1] - y[1]
c1[6] = Interval.POS_REALS
print("c1 = ", c1)
ctc.contract(c1)
print(" c1 = ", c1)

c3 = IntervalVector(7)
c3[0] = ddx[0] & Interval.NEG_REALS
c3[1] = ddx[1] & Interval.NEG_REALS
c3[2] = dx[0] & Interval.POS_REALS
c3[3] = dx[1] & Interval.POS_REALS
c3[4] = x[0] - y[0]
c3[5] = x[1] - y[1]
c3[6] = Interval.POS_REALS
print("c3 = ", c3)
ctc.contract(c3)
print(" c3 = ", c3)

c2 = IntervalVector(7)
c2[0] = ddx[0] & Interval.POS_REALS
c2[1] = ddx[1] & Interval.POS_REALS
c2[2] = dx[0] & Interval.NEG_REALS
c2[3] = dx[1] & Interval.NEG_REALS
c2[4] = x[0] - y[0]
c2[5] = x[1] - y[1]
c2[6] = Interval.POS_REALS
print("c2 = ", c2)
ctc.contract(c2)
print(" c2 = ", c2)

c4 = IntervalVector(7)
c4[0] = ddx[0] & Interval.POS_REALS
c4[1] = ddx[1] & Interval.POS_REALS
c4[2] = dx[0] & Interval.POS_REALS
c4[3] = dx[1] & Interval.POS_REALS
c4[4] = x[0] - y[0]
c4[5] = x[1] - y[1]
c4[6] = Interval.POS_REALS
print("c4 = ", c4)
ctc.contract(c4)
print(" c4 = ", c4)

OUT_contract[0] = (c1[4] | c2[4] | c3[4] | c4[4]) - IN[0]
OUT_contract[1] = (c1[5] | c2[5] | c3[5] | c4[5]) - IN[1]

############

in_lb = IntervalVector(IN.lb())
vect_in_lb = f.eval_vector(in_lb)
# a1=Interval(0.5*vect_d[out_degenerated].lb())
# b1=Interval(vect_in_lb[out_degenerated].lb())
# c1=in_lb[out_degenerated]-OUT[out_degenerated].lb()
	
# # print("----")
# # print("a1 = ", a1)
# # print("b1 = ", b1)
# # print("c1 = ", c1)
# # print("vect_in_lb = ", vect_in_lb)

# if(a1 != Interval(0)):
# 	t1 = Interval((((-b1-sqrt((pow(b1, 2)-4*a1*c1)))/(2*a1)) & Interval.POS_REALS).lb())
# 	t2 = Interval((((-b1+sqrt((pow(b1, 2)-4*a1*c1)))/(2*a1)) & Interval.POS_REALS).lb())
# 	# print("t1=", t1)
# 	# print("t2=", t2)
# 	if((not t1.is_empty()) and (not t2.is_empty())):
# 		t = min(t1, t2)
# 	elif(not t1.is_empty()):
# 		t = t1
# 	else:
# 		t = t2
# 	# print("t=", t)
# 	if(Interval.ZERO.is_subset(t)):
# 		y |= OUT[1-out_degenerated]# & in_lb[1-out_degenerated]
# 	y |= 0.5*vect_d[1-out_degenerated].lb()*(t**2)+vect_in_lb[1-out_degenerated].lb()*t+in_lb[1-out_degenerated].lb()
# else:
# 	if(b1 == Interval.ZERO):
# 		y |= OUT[1-out_degenerated]
# 	t = Interval((-c1/b1 & Interval.POS_REALS).lb())
# 	# print("t=", t)
# 	if(Interval.ZERO.is_subset(t)):
# 		y |= OUT[1-out_degenerated] & in_lb[1-out_degenerated]
# 	y |= 0.5*vect_d[1-out_degenerated].lb()*(t**2)+vect_in_lb[1-out_degenerated].lb()*t+in_lb[1-out_degenerated].lb()


# # print("y=", y)

in_ub = IntervalVector(IN.ub())
vect_in_ub = f.eval_vector(in_ub)
# a1=Interval(0.5*vect_d[out_degenerated].ub())
# b1=Interval(vect_in_ub[out_degenerated].ub())
# c1=in_ub[out_degenerated]-OUT[out_degenerated].lb()

# # print("----")
# # print("a1 = ", a1)
# # print("b1 = ", b1)
# # print("c1 = ", c1)

# if(a1 != Interval(0)):
# 	t1 = Interval((((-b1-sqrt((pow(b1, 2)-4*a1*c1)))/(2*a1)) & Interval.POS_REALS).lb())
# 	t2 = Interval((((-b1+sqrt((pow(b1, 2)-4*a1*c1)))/(2*a1)) & Interval.POS_REALS).lb())
# 	# print("t1=", t1)
# 	# print("t2=", t2)
# 	if((not t1.is_empty()) and (not t2.is_empty())):
# 		t = min(t1, t2)
# 	elif(not t1.is_empty()):
# 		t = t1
# 	else:
# 		t = t2
# 	# print("t=", t)
# 	if(Interval.ZERO.is_subset(t)):
# 		y |= OUT[1-out_degenerated] #& in_ub[1-out_degenerated]
# 	y |= 0.5*vect_d[1-out_degenerated].ub()*(t**2)+vect_in_ub[1-out_degenerated].ub()*t+in_ub[1-out_degenerated].ub()
# else:
# 	if(b1 == Interval.ZERO):
# 		y |= OUT[1-out_degenerated]
# 	t = Interval((-c1/b1 & Interval.POS_REALS).lb())
		
# 	# print("t=", t, -c1/b1)
# 	if(Interval.ZERO.is_subset(t)):
# 		y |= OUT[1-out_degenerated] & in_ub[1-out_degenerated]
# 	y |= 0.5*vect_d[1-out_degenerated].ub()*(t**2)+vect_in_ub[1-out_degenerated].ub()*t+in_ub[1-out_degenerated].ub()

# OUT_contract[1-out_degenerated] &= y
# # print("OUT_contract = ", OUT_contract)

######################## Drawing ########################
vibes.beginDrawing()
vibes.newFigure("Test")
vibes.setFigureSize(512, 512)
vibes.axisLimits(pave[0].lb(), pave[0].ub(), pave[1].lb(), pave[1].ub())

########## Trajectories ##########

## Taylor 0

pt_lb_list_ub = []
pt_lb_list_lb = []
pt_ub_list_ub = []
pt_ub_list_lb = []

time = np.linspace(0, 2, 100)
for t in time:
	pt_x = vect[0]*t+in_lb[0]
	pt_y = vect[1]*t+in_lb[1]
	pt_lb_list_ub.append([pt_x.ub(), pt_y.ub()])
	pt_lb_list_lb.append([pt_x.lb(), pt_y.lb()])
	
	pt_x = vect[0]*t+in_ub[0]
	pt_y = vect[1]*t+in_ub[1]
	pt_ub_list_ub.append([pt_x.ub(), pt_y.ub()])
	pt_ub_list_lb.append([pt_x.lb(), pt_y.lb()])

vibes.drawLine(pt_lb_list_ub, "green")
vibes.drawLine(pt_lb_list_lb, "green")
vibes.drawLine(pt_ub_list_ub, "green")
vibes.drawLine(pt_ub_list_lb, "green")

## Taylor 1
pt_lb_list_ub = []
pt_lb_list_lb = []
pt_ub_list_ub = []
pt_ub_list_lb = []

time = np.linspace(0, 1, 100)
# print(vect_d)
for t in time:
	pt_x = 0.5*vect_d[0]*(t**2)+vect_in_lb[0]*t+in_lb[0]
	pt_y = 0.5*vect_d[1]*(t**2)+vect_in_lb[1]*t+in_lb[1]
	pt_lb_list_ub.append([pt_x.ub(), pt_y.ub()])
	pt_lb_list_lb.append([pt_x.lb(), pt_y.lb()])

	pt_x = 0.5*vect_d[0]*(t**2)+vect_in_ub[0]*t+in_ub[0]
	pt_y = 0.5*vect_d[1]*(t**2)+vect_in_ub[1]*t+in_ub[1]
	pt_ub_list_ub.append([pt_x.ub(), pt_y.ub()])
	pt_ub_list_lb.append([pt_x.lb(), pt_y.lb()])

	# pt_x_lb = 0.5*vect_d[0].lb()*(t**2)+vect_in_lb[0].lb()*t+in_lb[0].lb()
	# pt_y_lb = 0.5*vect_d[1].lb()*(t**2)+vect_in_lb[1].lb()*t+in_lb[1].lb()
	# pt_lb_list_lb.append([pt_x_lb, pt_y_lb])
	# pt_x_ub = 0.5*vect_d[0].ub()*(t**2)+vect_in_ub[0].ub()*t+in_ub[0].ub()
	# pt_y_ub = 0.5*vect_d[1].ub()*(t**2)+vect_in_ub[1].ub()*t+in_ub[1].ub()
	# pt_ub_list_ub.append([pt_x_ub, pt_y_ub])

vibes.drawLine(pt_lb_list_ub, "orange")
vibes.drawLine(pt_lb_list_lb, "orange")
vibes.drawLine(pt_ub_list_ub, "orange")
vibes.drawLine(pt_ub_list_lb, "orange")

######################## Functions ########################

def drawBox(x, color=''):
	vibes.drawBox(x[0].lb(), x[0].ub(), x[1].lb(), x[1].ub(), color)

def draw_vector_field(f):
	for x in np.arange(pave[0].lb(), pave[0].ub(), pave[0].diam()/10):
		for y in np.arange(pave[1].lb(), pave[1].ub(), pave[1].diam()/10):
			result = f.eval_vector(IntervalVector([[x, x], [y, y]]))
			factor = 0.5*(np.sqrt(result[0].ub()**2 + result[1].ub()**2))
			result = result / factor * (1/40.0*np.max((pave[0].diam(), pave[1].diam())))
			vibes.drawArrow([x, y], [x+result[0].mid(), y+result[1].mid()], pave[0].diam()/100, 'black[black]')
			# vibes.drawArrow([x, y], [x+f_n_numpy2(x, y)[0], y+f_n_numpy2(x, y)[1]], 0.01, 'grey[grey]')

			# vibes.drawArrow([x, y], [x+f(x, y)[0].mid(), y+f(x, y)[1].mid()], 0.01, 'black[black]')

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

draw_vector_field(f)

