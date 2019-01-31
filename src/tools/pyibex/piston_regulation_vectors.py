from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm

from pyibex import *
import numpy as np

g = 9.81
rho = 1025.0
m = 8.8
diam = 0.24
alpha = 1.432e-06

x2_target = 0.5
beta = 0.04/(np.pi/2.0)
l1 = 0.1
l2 = 0.1

Cf = np.pi*np.power(diam/2.0,2)

A = g*rho/m
B = 0.5*rho*Cf/m

def y(x1, x2):
	return (x1+beta*np.arctan(x2_target-x2))

def dx1(x1, x2, x3):
	return (-A*(x3-alpha*x2)-B*np.abs(x1)*x1)

def e(x2):
	return (x2_target-x2)

def D(x2):
	return (1+np.power(e(x2), 2))

def dy(x1, x2, x3):
	return (dx1(x1, x2, x3)-beta*x1/D(x2))

def u(x1, x2, x3):
	return  (1.0/A*(l1*dy(x1, x2, x3)+l2*y(x1, x2)+beta*(2*e(x2)*np.power(x1,2)-dx1(x1, x2, x3)*D(x2))/(np.power(D(x2),2))-2*B*np.abs(x1)*x1)+alpha*x1)

def f(x1, x2, x3):
	return [-A*(x3-alpha*x2)-B*np.abs(x1)*x1, x1, u(x1, x2, x3)]

n_x1 = 15
n_x2 = 1
n_x3 = 15

x1_ls = np.linspace(-0.02, 0.02, n_x1)	# Velocity
x2_ls = 0.1*np.ones(1)
x3_ls = np.linspace(-1.7e-4/2.0, 1.7e-4/2.0, n_x3)

x1, x2, x3 = np.meshgrid(x1_ls, x2_ls, x3_ls, indexing='ij')

vec_x1 = np.zeros((n_x1, n_x2, n_x3))
vec_x2 = np.zeros((n_x1, n_x2, n_x3))
vec_x3 = np.zeros((n_x1, n_x2, n_x3))

for i in range(0, n_x1):
	for j in range(0, n_x2):
		for k in range(0, n_x3):
			tmp = f(x1[i, j, k], x2[i, j, k], x3[i, j, k])
			vec_x1[i, j, k] = tmp[0]
			vec_x2[i, j, k] = tmp[1]
			vec_x3[i, j, k] = tmp[2] 


fig = plt.figure()
ax = fig.gca(projection='3d')
ax.set_xlabel('x1')
ax.set_ylabel('x2')
ax.set_zlabel('x3')
ax.quiver(x1, x2, x3, vec_x1, vec_x2, vec_x3,length=1e-3, normalize=True)
print(x2)

# ax.set_xlim([x1_ls[0], x1_ls[-1]])
# ax.set_ylim([x2_ls[0], x2_ls[-1]])
# ax.set_zlim([x3_ls[0], x3_ls[-1]])

# Plot a basic wireframe.
# ax.plot_wireframe(x1, x3, x2, rstride=10, cstride=10)

fig1, ax1 = plt.subplots()
ax1.quiver(x1[:,0,:], x3[:,0,:], vec_x1[:,0,:], vec_x3[:,0,:])


# fig2 = plt.figure()
# ax2 = fig2.gca(projection='3d')
# ax2.plot_surface(x1[:,0,:], x3[:,0,:], np.zeros((n_x1, n_x3)))
# ax2.plot_surface(x1[:,0,:], x3[:,0,:], vec_x2[:,0,:], cmap=cm.coolwarm)
# ax2.set_xlabel('x1')
# ax2.set_ylabel('x3')
# ax2.set_zlabel('x2')

plt.show()


print(f(-0.1, 0.3, -1.7e-4/2.0))