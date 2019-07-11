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

x, y = np.meshgrid(np.arange(-2.0, 2.0, 0.1),
                   np.arange(-2.0, 2.0, 0.1))

# u = y
# v = ((8.0/25.0*x**5-4.0/3.0*x**3+4.0/5.0*x)-0.3*y)

# u = -y
# v = -(-x-0.1*x**2)

u = y
v = -0.1*(20*x**3-10*x+5*y)

# v = -(x**2+y**2-1)*y-x

# v = -((1-x**2)*y-x)

N = np.sqrt(u**2+v**2)
u1, v1 = u/N, v/N
# u = u/((u**2+y**2))
# v = v/((u**2+y**2))

ax.quiver(x, y, u1, v1)
circle=plt.Circle((-0.5,0),1.0, color='r', fill=False)
ax.add_artist(circle)

plt.xlabel(r'$x_1$')
plt.ylabel(r'$x_2$')

plt.show()