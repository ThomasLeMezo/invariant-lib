from pyibex import *
from vibes import *
import numpy as np
# from math import *

def f(t, a, b, c):
	return a*(t**2)+b*t+c

def drawBox(x, color=''):
	vibes.drawBox(x[0].lb(), x[0].ub(), x[1].lb(), x[1].ub(), color)

def drawPolygone(a0, b0, c0, a1, b1, c1, t, color=''):
	pt_list = []
	time = np.linspace(t.lb(), t.ub(), 1000)
	for i in time:
		pt_x = f(i, a0, b0, c0)
		pt_y = f(i, a1, b1, c1)
		pt_list.append([pt_x, pt_y])

	vibes.drawLine(pt_list, color)

# def compute_summit(a0, b0, a1, b1):
# 	t_s=-(a0*b0+a1*b1)/(2*(a0**2+a1**2))
# 	s = IntervalVector(2)
# 	s[0] = f(t_s, a[0], b[0], c[0])
# 	s[1] = f(t_s, a[1], b[1], c[1])
# 	return s

def draw_trajectories(a, b, c, time_step):
	
	
	drawPolygone(a[0].lb(), b[0].ub(), c[0].lb(), a[1].lb(), b[1].ub(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].ub(), c[0].ub(), a[1].lb(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].lb(), a[1].ub(), b[1].lb(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].ub(), a[1].ub(), b[1].lb(), c[1].ub(), time_step, "red")
	
	drawPolygone(a[0].lb(), b[0].ub(), c[0].lb(), a[1].ub(), b[1].lb(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].ub(), c[0].ub(), a[1].ub(), b[1].lb(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].lb(), a[1].lb(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].ub(), a[1].lb(), b[1].ub(), c[1].lb(), time_step, "red")

	# c[0].lb(), c[1].ub()
	drawPolygone(a[0].ub(), b[0].ub(), c[0].lb(), a[1].lb(), b[1].lb(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].lb(), c[0].lb(), a[1].ub(), b[1].ub(), c[1].ub(), time_step, "red")
	# c[0].lb(), c[1].lb()
	drawPolygone(a[0].ub(), b[0].ub(), c[0].lb(), a[1].ub(), b[1].ub(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].lb(), c[0].lb(), a[1].lb(), b[1].lb(), c[1].lb(), time_step, "red")
	
	# c[1].ub(), c[1].lb()
	drawPolygone(a[0].ub(), b[0].ub(), c[0].ub(), a[1].lb(), b[1].lb(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].lb(), c[0].ub(), a[1].ub(), b[1].ub(), c[1].lb(), time_step, "red")

	# c[1].ub(), c[1].ub()
	drawPolygone(a[0].ub(), b[0].ub(), c[0].ub(), a[1].ub(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].lb(), c[0].ub(), a[1].lb(), b[1].lb(), c[1].ub(), time_step, "red")

def compute_box(t, a, b, c):
	x = a[0]*t**2+b[0]*t+c[0]
	y = a[1]*t**2+b[1]*t+c[1]
	return IntervalVector([x, y])

def compute_t(x, a, b, c, t):
	if(not a == Interval.ZERO):
		t1 = (-b+sqrt(b**2-4*a*c))/(2*a) & Interval.POS_REALS & t
		t2 = (-b-sqrt(b**2-4*a*c))/(2*a) & Interval.POS_REALS & t	

		if(not (t2 & Interval.POS_REALS).is_empty()):
			if(t1.lb()<=t2.ub()):
				return (t2 | t1) & t
			else:
				return t2 & t
		else:
			return t1 & t
	else:
		if(not b == Interval.ZERO):
			print("-c/b = ", -c/b)
			return -c/b & Interval.POS_REALS & t
		else:
			return Interval.POS_REALS & t
			
def contract_box(box, a, b, c, sens, t):
	box_tmp = IntervalVector(2, Interval.EMPTY_SET)
	if(sens==1): ## contract y
		c1 = c[1][0]-box[1]
		t_y = compute_t(box[1], a[1], b[1], c1.lb(), t)
		box_tmp |= compute_box(t_y, a, b, c)
		t_y = compute_t(box[1], a[1], b[1], c1.ub(), t)
		box_tmp |= compute_box(t_y, a, b, c)

		c1 = c[1][1]-box[1]
		t_y = compute_t(box[1], a[1], b[1], c1.lb(), t)
		box_tmp |= compute_box(t_y, a, b, c)
		t_y = compute_t(box[1], a[1], b[1], c1.ub(), t)
		box_tmp |= compute_box(t_y, a, b, c)
	else:
		c1 = c[0][0]-box[0]
		t_x = compute_t(box[0], a[0], b[0], c1.lb(), t)
		box_tmp |= compute_box(t_x, a, b, c)
		t_x = compute_t(box[0], a[0], b[0], c1.ub(), t)
		box_tmp |= compute_box(t_x, a, b, c)

		c1 = c[0][1]-box[0]
		t_x = compute_t(box[0], a[0], b[0], c1.lb(), t)
		box_tmp |= compute_box(t_x, a, b, c)
		t_x = compute_t(box[0], a[0], b[0], c1.ub(), t)
		box_tmp |= compute_box(t_x, a, b, c)
	return box_tmp

def test_case(pave, box, a, b, c, name):
	vibes.newFigure(name)
	vibes.setFigureSize(512, 512)
	vibes.axisLimits(-0.5, 2, -0.5, 1.5)
	print("#### ", name, "####")

	sens = 0
	if(c[0].is_degenerated()):
		sens = 1
	print("sens = ", sens)

	drawBox(pave, "black")
	box_contract = IntervalVector(2, Interval.EMPTY_SET)
	k=0
	for i1 in range(2):
		for i2 in range(2):
			for i3 in range(2):
				for i4 in range(2):
					k+=1
					a_tmp = IntervalVector([Interval(a[0][i1]), Interval(a[1][i2])])
					b_tmp = IntervalVector([Interval(b[0][i3]), Interval(b[1][i4])])
					box_contract |= contract_box(box, a_tmp, b_tmp, c, sens, Interval.POS_REALS)
	box_contract &= box
	print("box_contract = ", box_contract)
	drawBox(box_contract, "green[]")

	# drawBox(compute_box(t_inter, a, b, c), "blue")

	drawBox(c, "blue")

	time_step = Interval([0, 2])
	draw_trajectories(a, b, c, time_step)
	# vibes.drawCircle(s_x, s_y, 0.1, "blue")

######################## Main ########################
vibes.beginDrawing()

pave = IntervalVector([[0, 1], [0, 1]])
box = IntervalVector([[0, 1], [1, 1]])

# a = IntervalVector([[1, 1], [1, 2]])
# b = IntervalVector([[1, 1], [0, 0]])
# c = IntervalVector([[0, 0], [0, 0]])
# test_case(pave, box, a, b, c, "Test 1")

# a = IntervalVector([[0.8, 1], [0.8, 1]])
# b = IntervalVector([[0.5, 0.8], [0.5, 0.8]])
# c = IntervalVector([[0.5, 0.7], [0, 0]])
# test_case(pave, box, a, b, c, "Test 2")

########### Not optimal ! ####################
a = IntervalVector([[0.5, 1.0], [1, 1]])
b = IntervalVector([[0.5, 1], [0.5, 1]])
c = IntervalVector([[0.5, 0.7], [0, 0]])
test_case(pave, box, a, b, c, "Test 3") 
##############################################

# a = IntervalVector([[0.5, 1.0], [1, 1]])
# b = IntervalVector([[0.5, 1], [0.5, 1]])
# c = IntervalVector([[0, 0], [0.25, 0.5]])
# test_case(pave, box, a, b, c, "Test 4")

# a = IntervalVector([[-1.0, -0.5], [1, 1]])
# b = IntervalVector([[0.5, 1], [0.5, 1]])
# c = IntervalVector([[0, 0], [0.25, 0.5]])
# test_case(pave, box, a, b, c, "Test 5")

# a = IntervalVector([[-1.0, 0], [0.0, 1]])
# b = IntervalVector([[0.5, 1], [0.5, 1]])
# c = IntervalVector([[0, 0], [0.25, 0.5]])
# test_case(pave, box, a, b, c, "Test 6")

# a = IntervalVector([[-0.5, 0.5], [-0.5, 0.5]])
# b = IntervalVector([[0.5, 1], [0.5, 1]])
# c = IntervalVector([[0, 0], [0.25, 0.5]])
# test_case(pave, box, a, b, c, "Test 7")

# a = IntervalVector([[-1, 1], [-1, 1]])
# b = IntervalVector([[-1, 1], [-1, 1]])
# c = IntervalVector([[0, 0], [0.25, 0.5]])
# test_case(pave, box, a, b, c, "Test 8")