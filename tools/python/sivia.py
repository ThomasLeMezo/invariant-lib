# Solve f(x) = 0

from pyibex import *
from math import *
from paraview.simple import *

position = IntervalVector(3, [-4, 4])
zero = IntervalVector(3, Interval.ZERO)
f = Function("x", "y", "z", "( sin(y)-0.1998*x, sin(z)-0.1998*y, sin(x)-0.1998*z)")
bisector  = LargestFirst()
list_box = [position]

for k in range(0, 40):
	list_tmp = list_box
	list_box = []
	while len(list_tmp)>0:
		box = list_tmp.pop()
		b = bisector.bisect(box)

		result0 = f.eval_vector(b[0])
		if(zero.is_subset(result0)):
			list_box.append(b[0])

		result1 = f.eval_vector(b[1])
		if(zero.is_subset(result1)):
			list_box.append(b[1])

# print(list_box)
print(len(list_box))

### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()
renderView1 = GetActiveViewOrCreate('RenderView')

for b in list_box:
	box1 = Box()

	# Properties modified on box1
	box1.XLength = b[0].diam()
	box1.YLength = b[1].diam()
	box1.ZLength = b[2].diam()
	box1.Center = [b[0].mid(), b[1].mid(), b[2].mid()]

	# show data in view
	box1Display = Show(box1, renderView1)
	# trace defaults for the display properties.
	box1Display.ColorArrayName = [None, '']
	box1Display.GlyphType = 'Arrow'
	box1Display.SetRepresentationType('Surface With Edges')



