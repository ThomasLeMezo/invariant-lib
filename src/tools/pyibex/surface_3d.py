'''
=================
3D wireframe plot
=================

A very basic demonstration of a wireframe plot.
'''

from mpl_toolkits.mplot3d import axes3d
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
import numpy as np


fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

x, y = np.meshgrid(np.arange(-6.0, 6.0, 0.2),
                      np.arange(-6.0, 6.0, 0.2))

z = 2.0*np.exp((np.square(x+2)+np.square(y+2))/-10.0) + 2.0*np.exp((np.square(x-2)+np.square(y-2))/-10.0) - 10.0

# # Plot a basic wireframe.
# ax.plot_wireframe(x, y, z, rstride=1, cstride=1)

# plt.show()


# Plot the surface.
surf = ax.plot_surface(x, y, z, cmap=cm.coolwarm,
                       linewidth=0, antialiased=False)

# Customize the z axis.
# ax.set_zlim(-1.01, 1.01)
# ax.zaxis.set_major_locator(LinearLocator(10))
# ax.zaxis.set_major_formatter(FormatStrFormatter('%.02f'))

# Add a color bar which maps values to colors.
fig.colorbar(surf, shrink=0.5, aspect=5)

plt.show()

