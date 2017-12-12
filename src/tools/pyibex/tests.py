#!/usr/bin/python3.4

# from pyibex import *

# a = IntervalVector([[0, 1], [0, 1]])
# b = IntervalVector([[0, 0],[0, 0]])

# print(a.diff(b))

x_max = 583
y_max = 753

x_code = 150
y_code = 180

x = x_code*2 # 300
y = y_code*2 # 300
t = 4*2

grid = x*y*t * 2
print('grid size = ',grid)

grid_data = grid * (5 * 16) / (8e6) # In Mo
print('grid data = ', grid_data, 'Mo')


