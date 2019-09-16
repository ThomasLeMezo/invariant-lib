from pyibex import *
from vibes import *
import numpy as np




f_sep_outer = Function("x1", "x2", "x1^2+(x2+2)^2-1")
s_outer = SepFwdBwd(f_sep_outer, LEQ)
f_sep_outer_bis = Function("x1", "x2", "(x1-1)^3+(x2+3)^2-1")
s_outer_bis = SepFwdBwd(f_sep_outer_bis, LEQ)
sep_u = SepUnion([s_outer, s_outer_bis])

pave_in = IntervalVector([[0, 0.03125], [-3.0546875, -2.98046875]])
pave_out = IntervalVector([[0, 0.03125], [-3.0546875, -2.98046875]])

box_in = IntervalVector([[0, 0], [-3.0546875, -2.98046875]])
box_out = IntervalVector([[0, 0], [-3.0546875, -2.98046875]])

sep_i = SepNot(sep_u)
print(pave_in)
print(pave_out)
sep_i.separate(pave_in, pave_out)
print(pave_in)
print(pave_out)

box_in = IntervalVector([[0, 0], [-3.0546875, -2.98046875]])
box_out = IntervalVector([[0, 0], [-3.0546875, -2.98046875]])
print(box_in)
print(box_out)
sep_u.separate(box_in, box_out)
print(box_in)
print(box_out)