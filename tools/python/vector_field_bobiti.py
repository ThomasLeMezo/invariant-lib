import matplotlib.pyplot as plt
import numpy as np

from matplotlib import rc
rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:
#rc('font',**{'family':'serif','serif':['Palatino']})
rc('text', usetex=True)

plt.rc('text', usetex=True)
plt.rc('font', family='serif')

fig = plt.figure()
ax = fig.gca()

x, y = np.meshgrid(np.arange(-1.0, 1.0, 0.1),
                   np.arange(-1.3, 1.3, 0.1))

u = 0.5*x+x**2-y**2
v = -0.5*y+x**2

## (0.0, 0.0), (0.59, 0.23), (1.11, 0.83), (-1.71, 1.64)

N = np.sqrt(u**2+v**2)
u1, v1 = u/N, v/N
# u = u/((u**2+y**2))
# v = v/((u**2+y**2))

ax.streamplot(x, y, u1, v1)
# circle=plt.Circle((-0.5,0),1.0, color='r', fill=False)
# ax.add_artist(circle)

plt.xlabel(r'$x_1$')
plt.ylabel(r'$x_2$')

plt.show()