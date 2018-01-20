from pyibex import *
from vibes import *
import numpy as np

f_poly = Function("x[2]", "a", "b", "c", "(a*((x[0])^2)+b*(x[0])+c-x[1])")
ctc = CtcFwdBwd(f_poly)

def drawBox(x, color=''):
	vibes.drawBox(x[0].lb(), x[0].ub(), x[1].lb(), x[1].ub(), color)

def drawPolygone(a, b, c, x, color=''):
	pt_x = np.arange(x.lb(), x.ub(), x.diam()/1000)
	pt_list=[]
	for t in pt_x:
		pt_list.append([t, a*t**2+b*t+c])
	vibes.drawLine(pt_list, color)

def contract(x, y, a, b, c):
	box1 = IntervalVector(5)
	box1[0] = x
	box1[1] = y
	box1[2] = a
	box1[3] = b
	box1[4] = c
	ctc.contract(box1)
	return IntervalVector([box1[0], box1[1]])

def draw_summit(a, b, c, color):
	x_s = (-b/(2*a))
	y_s = (-b**2/(4*a)+c)
	vibes.drawCircle(x_s, c-y_s, 0.1, color)


a = Interval(0.001, 2)
b = Interval(0, 1)
c = Interval(0)

box = IntervalVector([[-10, 10], [-10, 10]])

x = box[0]
y = box[1]

vibes.beginDrawing()
vibes.newFigure("Polygon")
vibes.setFigureSize(512, 512)
vibes.axisLimits(-10, 10, -2, 15)

# drawPolygone(a.mid(), b.mid(), c.mid(), x)
drawBox(box, "green")

# drawPolygone(a.ub(), b.ub(), c.lb(), x, "blue")
# drawPolygone(a.ub(), b.ub(), c.ub(), x, "blue")
# drawPolygone(a.ub(), b.lb(), c.lb(), x, "blue")
# drawPolygone(a.ub(), b.lb(), c.ub(), x, "blue")

# drawPolygone(a.lb(), b.ub(), c.mid(), x, "black")
# drawPolygone(a.lb(), b.lb(), c.mid(), x, "black")
# drawPolygone(a.lb(), b.mid(), c.mid(), x, "black")

for b_val in np.linspace(-5, 5.0, 200):
	draw_summit(a.lb(), b_val, c.mid(), "black")


## Summit

# Case a>=0

### New polynome
a_ub = a.ub()
x_s = (-b/(2*a_ub))
y_s = (-b**2/(4*a_ub)+c)
print("center = ", x_s, y_s)
# vibes.drawCircle(x_s.mid(), y_s.mid(), 0.1, "black[black]")
# drawPolygone(a.ub(), b.mid(), c.mid(), x, "black")

a1 = Interval(a_ub)
b1 = b+2*a_ub*x_s
c1 = a_ub*(x_s**2)+b*x_s+c-y_s

box1 = contract((x - x_s) & Interval.POS_REALS, (y - y_s), a1, Interval(b1.lb()), Interval(c1.lb()))

box_r1 = IntervalVector(box)
box_r1[0] = box1[0] + x_s# & x
box_r1[1] = box1[1] + y_s# & y
print("box_r1 = ", box_r1)
drawBox(box_r1, "blue")

###############################

a_lb = a.lb()
x_s = (-b/(2*a_lb))
y_s = (-b**2/(4*a_lb)+c)
print("center = ", x_s, y_s)
# vibes.drawCircle(x_s.mid(), y_s.mid(), 0.1, "black[black]")
# drawPolygone(a.lb(), b.mid(), c.mid(), x, "black")

a1 = a_lb
b1 = b+2*a_lb*x_s
c1 = a_lb*x_s**2+b*x_s+c-y_s

box3 = IntervalVector(5)
box3[0] = (x - x_s) & Interval.POS_REALS
box3[1] = (y - y_s)
box3[2] = Interval(a1)
box3[3] = b1
box3[4] = c1

box4 = IntervalVector(5)
box4[0] = (x - x_s) & Interval.NEG_REALS
box4[1] = (y - y_s)
box4[2] = Interval(a1)
box4[3] = b1
box4[4] = c1

ctc.contract(box3)
ctc.contract(box4)


box_r2 = IntervalVector(box)
box_r2[0] = ((box3[0] | box4[0]) + x_s)# & x
box_r2[1] = ((box3[1] | box4[1]) + y_s)# & y
print("box_r2 = ", box_r2)
# drawBox(box_r2, "red")

box[0] = box_r1[0] | box_r2[0]
box[1] = box_r1[1] | box_r2[1]
drawBox(box, "red")

