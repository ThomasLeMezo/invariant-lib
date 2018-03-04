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
	x_s = -b/(2*a)
	y_s = -(b**2-4*a*c)/(4*a)
	vibes.drawCircle(x_s, y_s, 0.1, color)

def contract_box(a, b, c, box):
	x = box[0]
	y = box[1]

	box_droite = IntervalVector(2, Interval.EMPTY_SET)
	if((Interval.ZERO).is_subset(a)):
		print("zero subset of a")

		## Case a+
		if(a.ub()>0):
			print("case a ub")
			y_a = y & ((b*x + c) | Interval(y.ub()))
			x_a = x
			if(b is not Interval.ZERO):
				x_a_tmp = Interval.EMPTY_SET
				if(not (b & Interval.POS_REALS).is_empty()):
					x_a_tmp |= ((y_a - c)/b | Interval(x.lb()))
				if(not (b & Interval.NEG_REALS).is_empty()):
					x_a_tmp |= ((y_a - c)/b | Interval(x.ub()))
				x_a &= x_a_tmp
			else:
				x_a.set_empty()
				y_a = c
			y_a &= ((b*x + c) | Interval(y.ub()))
			box_droite[0] |= x_a
			box_droite[1] |= y_a

		if(a.lb()<0):
			print("case a lb")
			y_a = y & ((b*x + c) | Interval(y.lb()))
			x_a = x
			if(b is not Interval.ZERO):
				x_a_tmp = Interval.EMPTY_SET
				if(not (b & Interval.POS_REALS).is_empty()):
					x_a_tmp |= ((y_a - c)/b | Interval(x.ub()))
				if(not (b & Interval.NEG_REALS).is_empty()):
					x_a_tmp |= ((y_a - c)/b | Interval(x.lb()))
				x_a &= x_a_tmp
			else:
				x_a.set_empty()
				y_a = c
			y_a &= ((b*x + c) | Interval(y.lb()))
			box_droite[0] |= x_a
			box_droite[1] |= y_a
	box_contract = box_droite

	# Else
	if(not a.is_subset(Interval.ZERO)):
		print("a = ", a)
		# Summit :
		s = IntervalVector([-b/(2*a), -(b**2-4*a.ub()*c)/(4*a.ub())])
		s = s | IntervalVector([-b/(2*a), -(b**2-4*a.lb()*c)/(4*a.lb())])
		s &= box
		print("summit = ", s)
		print(b**2-4*a*c)
		print(4*a)

		draw_summit(a.mid(), b.mid(), c.mid(), "blue")
		drawBox(s, "red")

		# y Intersections
		y_right = (a*(x.ub())**2+b*x.ub()+c) & y
		y_left = (a*(x.lb())**2+b*x.lb()+c) & y

		right = IntervalVector([Interval(x.ub()), y_right])
		left = IntervalVector([Interval(x.lb()), y_left])	

		# x Intersections (racines)
		# y_lb case
		c1 = c-y.lb()
		x_bottom = ((-b-sqrt(b**2-4*a.lb()*c1))/(2*a.lb())) & x
		x_bottom |= ((-b-sqrt(b**2-4*a.ub()*c1))/(2*a.ub())) & x
		x_bottom |= ((-b+sqrt(b**2-4*a.lb()*c1))/(2*a.lb())) & x
		x_bottom |= ((-b+sqrt(b**2-4*a.ub()*c1))/(2*a.ub())) & x
		bottom = IntervalVector([x_bottom, Interval(y.lb())])

		# y_ub case
		c2 = c-y.ub()
		x_top = ((-b-sqrt(b**2-4*a.lb()*c2))/(2*a.lb())) & x
		x_top |= ((-b-sqrt(b**2-4*a.ub()*c2))/(2*a.ub())) & x
		x_top |= ((-b+sqrt(b**2-4*a.lb()*c2))/(2*a.lb())) & x
		x_top |= ((-b+sqrt(b**2-4*a.ub()*c2))/(2*a.ub())) & x
		top = IntervalVector([x_top, Interval(y.ub())])

		### Result

		if(right[1].is_empty()):
			right[0].set_empty()
		if(left[1].is_empty()):
			left[0].set_empty()
		if(top[0].is_empty()):
			top[1].set_empty()
		if(bottom[0].is_empty()):
			bottom[1].set_empty()

		box_contract = s | top | bottom | right | left

		drawBox(bottom, "red")
		drawBox(top, "red")
		drawBox(right, "red")
		drawBox(left, "red")

		# print("right = ", right)
		# print("left = ", left)
		# print("top = ", top)
		# print("bottom = ", bottom)

	# drawBox(box_contract)
	# print("box_contract = ", box_contract)

	return box_contract


a = Interval(-10, 10)
b = Interval(1)
c = Interval(1)

box = IntervalVector([[-10, 10], [-10, 10]])

vibes.beginDrawing()
vibes.newFigure("Polygon")
vibes.setFigureSize(512, 512)
vibes.axisLimits(-10, 10, -10, 10)

# drawPolygone(a.mid(), b.mid(), c.mid(), x)
drawBox(box, "green")

drawPolygone(a.ub(), b.ub(), c.ub(), box[0], "blue")
drawPolygone(a.ub(), b.lb(), c.ub(), box[0], "blue")
drawPolygone(a.ub(), b.ub(), c.lb(), box[0], "blue")
drawPolygone(a.ub(), b.lb(), c.lb(), box[0], "blue")
drawPolygone(a.lb(), b.ub(), c.ub(), box[0], "blue")
drawPolygone(a.lb(), b.lb(), c.ub(), box[0], "blue")
drawPolygone(a.lb(), b.ub(), c.lb(), box[0], "blue")
drawPolygone(a.lb(), b.lb(), c.lb(), box[0], "blue")

# drawPolygone(0.1, 1, 1, x, "grey")

# for b_val in np.linspace(-5, 5.0, 200):
# 	draw_summit(a.lb(), b_val, c.mid(), "black")

# Case 0 in a

print("box test = ", box)
box2 = IntervalVector(box)
box_contract = contract_box(a & Interval.POS_REALS, b, c, box)
print("box2 = ", box2)
box_contract |= contract_box(a & Interval.NEG_REALS, b, c, box2)

drawBox(box_contract)