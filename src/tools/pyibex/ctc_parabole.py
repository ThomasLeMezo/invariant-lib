from pyibex import *
from vibes import *
import numpy as np

f_poly = Function("x[2]", "a", "b", "c", "(a*x[0]^2+b*x[0]+c-x[1])")
ctc = CtcFwdBwd(f_poly)

def drawBox(x, color=''):
	vibes.drawBox(x[0].lb(), x[0].ub(), x[1].lb(), x[1].ub(), color)

def drawPolygone(a, b, c, x, color=''):
	pt_x = np.arange(x.lb(), x.ub(), x.diam()/1000)
	pt_list=[]
	for t in pt_x:
		pt_list.append([t, a*t**2+b*t+c])
	vibes.drawLine(pt_list, color)


a = Interval(0.5, 1)
b = Interval(1)
c = Interval(1)

box = IntervalVector([[-10, 10], [5, 10]])

x = box[0]
y = box[1]

vibes.beginDrawing()
vibes.newFigure("Polygon")
vibes.setFigureSize(512, 512)
# vibes.axisLimits(-1, 1, 0, 3.5)

drawPolygone(a.lb(), b.mid(), c.mid(), x)
drawPolygone(a.ub(), b.mid(), c.mid(), x)
drawPolygone(a.mid(), b.mid(), c.mid(), x)
drawBox(box, "green")

## Summit

# Case a>=0


x_s = (-b/(2*a))
y_s = (-b**2/(4*a)+c)
print(x_s, y_s)

vibes.drawCircle(x_s.mid(), y_s.mid(), 0.01, "red[red]")

### New polynome
a_ub = a.ub()
x_s = (-b/(2*a_ub))
y_s = (-b**2/(4*a_ub)+c)
print(x_s, y_s)
a1 = a_ub
b1 = b+2*a_ub*x_s
c1 = a_ub*x_s**2+b*x_s+c-y_s

box1 = IntervalVector(5)
box1[0] = (x + x_s) & Interval.POS_REALS
box1[1] = (y + y_s)
box1[2] = Interval(a1)
box1[3] = b1
box1[4] = c1

box2 = IntervalVector(5)
box2[0] = (x + x_s) & Interval.NEG_REALS
box2[1] = (y + y_s)
box2[2] = Interval(a1)
box2[3] = b1
box2[4] = c1

ctc.contract(box1)
ctc.contract(box2)
box[0] = ((box1[0] | box2[0]) - x_s)# & x
box[1] = ((box1[1] | box2[1]) - y_s)# & y

a_lb = a.lb()
x_s = (-b/(2*a_lb))
y_s = (-b**2/(4*a_lb)+c)
print(x_s, y_s)
a1 = a_lb
b1 = b+2*a_lb*x_s
c1 = a_lb*x_s**2+b*x_s+c-y_s

box3 = IntervalVector(5)
box3[0] = (x + x_s) & Interval.POS_REALS
box3[1] = (y + y_s)
box3[2] = Interval(a1)
box3[3] = b1
box3[4] = c1

box4 = IntervalVector(5)
box4[0] = (x + x_s) & Interval.NEG_REALS
box4[1] = (y + y_s)
box4[2] = Interval(a1)
box4[3] = b1
box4[4] = c1

ctc.contract(box3)
ctc.contract(box4)

box[0] = box[0] | ((box3[0] | box4[0]) - x_s)# & x
box[1] = box[1] | ((box3[1] | box4[1]) - y_s)# & y
drawBox(box, "red")

# drawPolygone(a1.mid(), b1.mid(), c1.mid(), x)