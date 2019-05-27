import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()
ax = fig.gca()

x, y = np.meshgrid(np.arange(-3.0, 3.0, 0.2),
                   np.arange(-3.0, 3.0, 0.2))

# u = y
# v = ((8.0/25.0*x**5-4.0/3.0*x**3+4.0/5.0*x)-0.3*y)

u = -y
v = -(-x-0.1*x**2)

# v = -(x**2+y**2-1)*y-x

# v = -((1-x**2)*y-x)

N = np.sqrt(u**2+v**2)
u1, v1 = u/N, v/N
# u = u/((u**2+y**2))
# v = v/((u**2+y**2))

ax.quiver(x, y, u1, v1)
circle=plt.Circle((-0.3,0),1.0, color='r', fill=False)
ax.add_artist(circle)
plt.show()