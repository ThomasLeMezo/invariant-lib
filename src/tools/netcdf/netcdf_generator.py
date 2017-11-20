import numpy as np
from netCDF4 import Dataset
import matplotlib.pyplot as plt

dataset = Dataset('test1.nc', 'w',  format='NETCDF4_CLASSIC')

ni = 584
nj = 754
scale_factor = 0.0006103702
n_i = dataset.createDimension('ni_u', ni)
n_j = dataset.createDimension('nj_u', nj)

valid_min = -32767
valid_max = 32767

u = dataset.createVariable('U', np.int16, ('nj_u', 'ni_u'), fill_value=-32768)
u.valid_min = valid_min
u.valid_max = valid_max
u.scale_factor = scale_factor

v = dataset.createVariable('V', np.int16, ('nj_u', 'ni_u'), fill_value=-32768)
v.valid_min = valid_min
v.valid_max = valid_max
v.scale_factor = scale_factor

# n_i_val = np.arange(0,10,1)
# n_j_val = np.arange(0,10,1)
# n_i[:] = n_i_val
# n_j[:] = n_j_val


X, Y = np.mgrid[0:nj, 0:ni]
U, V = -Y, X # 1m/s
# U = np.ones((nj, ni))*1.0
# V = X/(ni/2)
# plt.quiver(X, Y, U, V) #edgecolor='k', facecolor='None', linewidth=.5
# plt.show()
# U = np.ones((ni, nj))
# V = np.ones((ni, nj))

## Set Empty cells
# x_c = 210
# y_c = 400
# delta_p = 20
# delta_n = 2
# U[y_c+delta_n:y_c+delta_p,x_c+delta_n:x_c+delta_p]=-32768
# V[0:nj,0:ni]=1.0

print(V[401,209])
print(U[401,209])

print(U[209,401])
print(V[209,401])

u[:] = U
v[:] = V

dataset.close()