#!/usr/bin/python3.4

from pyibex import *

# seg_in=IntervalVector([[-1, 0], [0, 0]])
# seg_out=IntervalVector([[0, 0], [0, 1]])
# vec=IntervalVector([[0.1, 0.2],[-10, 10]])


seg_out = IntervalVector([[0,0.1],[0,1],[0,1]])
seg_in = IntervalVector([[,],[,],[,]])
vec = IntervalVector([[,],[,],[,]])

# seg_out = IntervalVector([[1, 2],[0,0]])
# seg_out[1] |= Interval.NEG_REALS
# seg_in = IntervalVector([[0, 1],[0,0]])
# vec = IntervalVector([[1.0, 2.0],[-0.5, 0.5]])

print('seg_in ', seg_in)
print('seg_out ', seg_out)

# v_diff = Interval.ALL_REALS.diff(vec[1])
# print(v_diff)

c = IntervalVector(seg_out-seg_in)
print('c=', c)
v = IntervalVector(vec)
alpha = Interval.POS_REALS

for i in range(0,2):
	# if(not c[i].is_degenerated()):
	alpha &= ((c[i]/(v[i] & Interval.POS_REALS)) & Interval.POS_REALS) | ((c[i]/(v[i] & Interval.NEG_REALS)) & Interval.POS_REALS)
	print(alpha)

c &= alpha*v;

seg_out &= c+seg_in; # FWD
seg_in &= seg_out - c

print('seg_in ', seg_in)
print('seg_out ', seg_out)

