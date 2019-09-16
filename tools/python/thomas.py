import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import odeint
from mpl_toolkits.mplot3d import Axes3D
from math import *

b = 0.1998

# rho = 13.0
# sigma = 10.0
# beta = 8.0 / 3.0

def f(state, t):
  x, y, z = state  # unpack the state vector
  return sin(y)-b*x, sin(z)-b*y, sin(x)-b*z  # derivatives

state0 = [4, 4, 0]
# state0 = [-1.0, 2.0, 5.0]
t = np.arange(0.0, 3000.0, 0.01)

states = odeint(f, state0, t)

fig = plt.figure()
ax = fig.gca(projection='3d')
ax.plot(states[:,0], states[:,1], states[:,2])
ax.set_xlabel('X axis')
ax.set_ylabel('Y axis')
ax.set_zlabel('Z axis')
plt.show()