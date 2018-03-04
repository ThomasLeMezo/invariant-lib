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

def compute_summit(a0, b0, a1, b1):
	t_s=-(a0*b0+a1*b1)/(2*(a0**2+a1**2))
	s = IntervalVector(2)
	s[0] = f(t_s, a[0], b[0], c[0])
	s[1] = f(t_s, a[1], b[1], c[1])
	return s


######################## Main ########################
vibes.beginDrawing()
vibes.newFigure("Test")
vibes.setFigureSize(512, 512)
vibes.axisLimits(-1, 1, -1, 1)

# a = IntervalVector([[1, 1], [1, 2]])
# b = IntervalVector([[1, 1], [0, 0]])
# c = IntervalVector([[0, 0], [0, 0]])

# a = IntervalVector([[0, 0.5], [1, 1]])
# b = IntervalVector([[1, 1], [1, 1]])
# c = IntervalVector([[0, 1], [0, 1]])

a = IntervalVector([[-0.5, 0.5], [1, 1]])
b = IntervalVector([[1, 1], [1, 1]])
c = IntervalVector([[0, 1], [0, 1]])

time_step = Interval([-2, 2])
# drawPolygone(a[0].lb(), b[0].lb(), c[0].lb(), a[1].lb(), b[1].lb(), c[1].lb(), time_step, "red")
drawPolygone(a[0].lb(), b[0].lb(), c[0].ub(), a[1].lb(), b[1].lb(), c[1].ub(), time_step, "red")
# drawPolygone(a[0].lb(), b[0].ub(), c[0].lb(), a[1].lb(), b[1].ub(), c[1].lb(), time_step, "red")
# drawPolygone(a[0].lb(), b[0].ub(), c[0].ub(), a[1].lb(), b[1].ub(), c[1].ub(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].lb(), c[0].lb(), a[1].ub(), b[1].lb(), c[1].lb(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].lb(), c[0].ub(), a[1].ub(), b[1].lb(), c[1].ub(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].ub(), c[0].lb(), a[1].ub(), b[1].ub(), c[1].lb(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].ub(), c[0].ub(), a[1].ub(), b[1].ub(), c[1].ub(), time_step, "red")

# drawPolygone(a[0].lb(), b[0].lb(), c[0].lb(), a[1].ub(), b[1].ub(), c[1].ub(), time_step, "red")
# drawPolygone(a[0].lb(), b[0].lb(), c[0].ub(), a[1].ub(), b[1].ub(), c[1].lb(), time_step, "red")
# drawPolygone(a[0].lb(), b[0].ub(), c[0].lb(), a[1].ub(), b[1].lb(), c[1].ub(), time_step, "red")
# drawPolygone(a[0].lb(), b[0].ub(), c[0].ub(), a[1].ub(), b[1].lb(), c[1].lb(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].lb(), c[0].lb(), a[1].lb(), b[1].ub(), c[1].ub(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].lb(), c[0].ub(), a[1].lb(), b[1].ub(), c[1].lb(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].ub(), c[0].lb(), a[1].lb(), b[1].lb(), c[1].ub(), time_step, "red")
# drawPolygone(a[0].ub(), b[0].ub(), c[0].ub(), a[1].lb(), b[1].lb(), c[1].lb(), time_step, "red")

s = compute_summit(a[0].lb(), b[0], a[1].lb(), b[1])
s |= compute_summit(a[0].lb(), b[0], a[1].ub(), b[1])
s |= compute_summit(a[0].ub(), b[0], a[1].lb(), b[1])
s |= compute_summit(a[0].ub(), b[0], a[1].ub(), b[1])

s_large = compute_summit(a[0], b[0], a[1], b[1])
drawBox(s_large, "orange")

# s_x = -b[0]/(2*a[0])
# s_y = f(-b[0]/(2*a[0]), a[0], b[0], c[0])

# print(s_x)
# print(s_y)
drawBox(s, "blue")
print("s=", s)
# vibes.drawCircle(s_x, s_y, 0.1, "blue")

