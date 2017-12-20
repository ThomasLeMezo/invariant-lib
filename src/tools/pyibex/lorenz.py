import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import odeint
from mpl_toolkits.mplot3d import Axes3D

rho = 29.0
sigma = 9.0
beta = 7.0 / 3.0

# rho = 13.0
# sigma = 10.0
# beta = 8.0 / 3.0

def f(state, t):
  x, y, z = state  # unpack the state vector
  return sigma * (y - x), (x * (rho - z) - y), (x * y - beta * z)  # derivatives

state0 = [100, 100, 5]
# state0 = [-1.0, 2.0, 5.0]
t = np.arange(0.0, 100.0, 0.001)

states = odeint(f, state0, t)

fig = plt.figure()
ax = fig.gca(projection='3d')
ax.plot(states[:,0], states[:,1], states[:,2])
ax.set_xlabel('X axis')
ax.set_ylabel('Y axis')
ax.set_zlabel('Z axis')
plt.show()


# from mpl_toolkits.mplot3d import axes3d
# import matplotlib.pyplot as plt
# import numpy as np

# rho = 13.0
# sigma = 10.0
# beta = 8.0 / 3.0

# fig = plt.figure()
# ax = fig.gca(projection='3d')

# x, y, z = np.meshgrid(np.arange(-3.0, 13.0, 2.0),
#                       np.arange(-1.0, 15.0, 2.0),
#                       np.arange(-1.0, 21.0, 2.0))

# u = sigma * (y - x)
# v = x * (rho - z) - y
# w = x * y - beta * z

# u=u*10.0
# v=v*10.0
# w=w*10.0

# ax.quiver(x, y, z, u, v, w, length=0.1)

# plt.show()