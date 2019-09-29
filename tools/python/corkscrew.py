
def f(state, t):
  x, y, z = state  # unpack the state vector
  return 1, z, -y-0.1*z  # derivatives



from mpl_toolkits.mplot3d import axes3d
import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()
ax = fig.gca(projection='3d')

x, y, z = np.meshgrid(np.arange(-1.0, 1.0, 0.2),
                      np.arange(-1.0, 1.0, 0.2),
                      np.arange(-1.0, 1.0, 0.2))

u = 1.0+0.0*z
v = z
w = -y-0.1*z

u=u
v=v
w=w

ax.quiver(x, y, z, u, v, w, length=0.1)

plt.show()