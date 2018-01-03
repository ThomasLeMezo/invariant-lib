from pyibex import *
from vibes import *
import numpy as np

def drawBox(x, color=''):
	vibes.drawBox(x[0].lb(), x[0].ub(), x[1].lb(), x[1].ub(), color)

def drawPolygone(a, b, c, x, color=''):
	pt_x = np.arange(x.lb(), x.ub(), x.diam()/100)
	pt_list=[]
	for t in pt_x:
		pt_list.append([t, a*t**2+b*t+c])
	vibes.drawLine(pt_list, color)

a = Interval(1)
b = Interval(1)
c = Interval(1)
x = Interval(-1, 1)

vibes.beginDrawing()
vibes.newFigure("Polygon")
vibes.setFigureSize(512, 512)
vibes.axisLimits(-1, 1, 0, 3.5)

drawPolygone(a.mid(), b.mid(), c.mid(), x)

## Summit
s_x = (-b/(2*a)).mid()
s_y = (-(b**2-4*a*c)/(4*a)).mid()

vibes.drawCircle(s_x, s_y, 0.01, "red[red]")

### New polynome

a1 = a
b1 = 2*b
c1 = 3*b*v *2/(4*a)+c
x1 = x + b/(2*a)

drawPolygone(a1.mid(), b1.mid(), c1.mid(), x1)