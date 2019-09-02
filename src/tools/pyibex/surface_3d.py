'''
=================
3D wireframe plot
=================

A very basic demonstration of a wireframe plot.
'''

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
import numpy as np


fig = plt.figure()
ax = fig.gca(projection='3d')

x, y = np.meshgrid(np.arange(-6.0, 6.0, 0.2),
                      np.arange(-6.0, 6.0, 0.2))

z = 2.0*np.exp((np.square(x+2)+np.square(y+2))/-10.0) + 2.0*np.exp((np.square(x-2)+np.square(y-2))/-10.0) - 10.0

# # Plot a basic wireframe.

surf = ax.plot_wireframe(x, y, z, rstride=1, cstride=1)
surf.set_linewidth(0.5)

cset = ax.contourf(x, y, z, zdir='z', offset=-10, cmap=cm.coolwarm)
fig.colorbar(cset, shrink=0.5, aspect=5)


ax.axis('equal')
ax.autoscale()

plt.show()
 