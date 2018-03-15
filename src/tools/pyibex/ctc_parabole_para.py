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
	drawPolygone(a[0].lb(), b[0].lb(), c[0].lb(), a[1].lb(), b[1].lb(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].lb(), c[0].ub(), a[1].lb(), b[1].lb(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].ub(), c[0].lb(), a[1].lb(), b[1].ub(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].ub(), c[0].ub(), a[1].lb(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].lb(), a[1].ub(), b[1].lb(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].ub(), a[1].ub(), b[1].lb(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].ub(), c[0].lb(), a[1].ub(), b[1].ub(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].ub(), c[0].ub(), a[1].ub(), b[1].ub(), c[1].ub(), time_step, "red")

	drawPolygone(a[0].lb(), b[0].lb(), c[0].lb(), a[1].ub(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].lb(), c[0].ub(), a[1].ub(), b[1].ub(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].ub(), c[0].lb(), a[1].ub(), b[1].lb(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].ub(), c[0].ub(), a[1].ub(), b[1].lb(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].lb(), a[1].lb(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].lb(), c[0].ub(), a[1].lb(), b[1].ub(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].ub(), c[0].lb(), a[1].lb(), b[1].lb(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].ub(), c[0].ub(), a[1].lb(), b[1].lb(), c[1].lb(), time_step, "red")

	drawPolygone(a[0].mid(), b[0].mid(), c[0].mid(), a[1].ub(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].mid(), b[0].mid(), c[0].ub(), a[1].ub(), b[1].ub(), c[1].mid(), time_step, "red")
	drawPolygone(a[0].mid(), b[0].ub(), c[0].mid(), a[1].ub(), b[1].mid(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].mid(), b[0].ub(), c[0].ub(), a[1].ub(), b[1].mid(), c[1].mid(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].mid(), c[0].mid(), a[1].mid(), b[1].ub(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].mid(), c[0].ub(), a[1].mid(), b[1].ub(), c[1].mid(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].ub(), c[0].mid(), a[1].mid(), b[1].mid(), c[1].ub(), time_step, "red")
	drawPolygone(a[0].ub(), b[0].ub(), c[0].ub(), a[1].mid(), b[1].mid(), c[1].mid(), time_step, "red")

	drawPolygone(a[0].lb(), b[0].lb(), c[0].lb(), a[1].mid(), b[1].mid(), c[1].mid(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].lb(), c[0].mid(), a[1].mid(), b[1].mid(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].mid(), c[0].lb(), a[1].mid(), b[1].lb(), c[1].mid(), time_step, "red")
	drawPolygone(a[0].lb(), b[0].mid(), c[0].mid(), a[1].mid(), b[1].lb(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].mid(), b[0].lb(), c[0].lb(), a[1].lb(), b[1].mid(), c[1].mid(), time_step, "red")
	drawPolygone(a[0].mid(), b[0].lb(), c[0].mid(), a[1].lb(), b[1].mid(), c[1].lb(), time_step, "red")
	drawPolygone(a[0].mid(), b[0].mid(), c[0].lb(), a[1].lb(), b[1].lb(), c[1].mid(), time_step, "red")
	drawPolygone(a[0].mid(), b[0].mid(), c[0].mid(), a[1].lb(), b[1].lb(), c[1].lb(), time_step, "red")

def compute_box(t, a, b, c):
	x = a[0]*t**2+b[0]*t+c[0]
	y = a[1]*t**2+b[1]*t+c[1]
	return IntervalVector([x, y])

######################## Main ########################
vibes.beginDrawing()
vibes.newFigure("Test")
vibes.setFigureSize(512, 512)
vibes.axisLimits(-0.5, 2, -0.5, 1.5)

pave = IntervalVector([[0, 1], [0, 1]])
drawBox(pave, "black")

# a = IntervalVector([[1, 1], [1, 2]])
# b = IntervalVector([[1, 1], [0, 0]])
# c = IntervalVector([[0, 0], [0, 0]])

a = IntervalVector([[0.8, 1], [0.8, 1]])
b = IntervalVector([[0.5, 0.8], [0.5, 0.8]])
c = IntervalVector([[0.5, 0.7], [0, 0]])

# a = IntervalVector([[0.5, 1.0], [1, 1]])
# b = IntervalVector([[0.5, 1], [0.5, 1]])
# c = IntervalVector([[0, 0], [0, 0]])

box = IntervalVector([[0, 1], [0.95, 1.05]])

def compute_t(x, a, b, c, t):
	c1 = c-x
	t1 = Interval.EMPTY_SET
	t2 = Interval.EMPTY_SET
	a_pos = a & Interval.POS_REALS
	a_neg = a & Interval.POS_REALS

	if(a.lb()!=0):
		t1 |= (-b+sqrt(b**2-4*a_pos*c1))/(2*a_pos) & Interval.POS_REALS & t
		t2 |= (-b-sqrt(b**2-4*a_pos*c1))/(2*a_pos) & Interval.POS_REALS & t
	
	if(a.ub()!=0):
		t1 |= (-b+sqrt(b**2-4*a_neg*c1))/(2*a_neg) & Interval.POS_REALS & t
		t2 |= (-b-sqrt(b**2-4*a_neg*c1))/(2*a_neg) & Interval.POS_REALS & t

	print("t1 = ", t1)
	print("t2 = ", t2)
	if(not (t2 & Interval.POS_REALS).is_empty()):
		if(t1.lb()<=t2.ub()):
			return (t2 | t1) & Interval.POS_REALS & t
		else:
			return t2 & t
	else:
		return t1 & t

def contract_box(box, a, b, c, t):
	t_x = compute_t(box[0], a[0], b[0], c[0], t)
	t_y = compute_t(box[1], a[1], b[1], c[1], t)
	t_inter = t_x & t_y
	print("t_inter=", t_inter)
	return [compute_box(t_inter, a, b, c) & box, t_inter]

[box_contract, t_inter] = contract_box(box, a, b, c, Interval.POS_REALS)
# [box_contract, t_inter] = contract_box(box_contract, a, b, c, t_inter)

print("box_contract = ", box_contract)
drawBox(box_contract, "green[green]")

drawBox(compute_box(t_inter, a, b, c), "blue")

drawBox(box, "orange")

drawBox(c,"blue")
time_step = Interval([0, 10])
draw_trajectories(a, b, c, time_step)
# vibes.drawCircle(s_x, s_y, 0.1, "blue")

