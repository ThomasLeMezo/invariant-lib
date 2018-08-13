#!/bin/python

from netCDF4 import Dataset
import numpy as np
import matplotlib.pyplot as plt
from pyproj import Proj, transform

path = "/home/lemezoth/workspaceQT/seabot-lib/data/"

file = "MARC_L1-MARS2D-FINIS250_20170709T0000Z_MeteoMF.nc"

rootgrp = Dataset(path+file, "r+", format="NETCDF4_CLASSIC")

# longitude_u_scale_factor = np.array(rootgrp.variables["longitude_u"].scale_factor, dtype=np.float64)
# longitude_u_add_offset = np.array(rootgrp.variables["longitude_u"].add_offset, dtype=np.float64)
# latitude_u_scale_factor = np.array(rootgrp.variables["latitude_u"].scale_factor, dtype=np.float64)
# latitude_u_add_offset = np.array(rootgrp.variables["latitude_u"].add_offset, dtype=np.float64)

longitude_u = np.array(rootgrp.variables["longitude_u"][:], dtype=np.float64)
latitude_u = np.array(rootgrp.variables["latitude_u"][:], dtype=np.float64)

u = np.array(rootgrp.variables["U"][0][:], dtype=np.float64)

# longitude_u = longitude_u_raw*longitude_u_scale_factor + longitude_u_add_offset
# latitude_u = latitude_u_raw*latitude_u_scale_factor + latitude_u_add_offset

# print("longitude_u_scale_factor = ", longitude_u_scale_factor)
# print("longitude_u_add_offset = ", longitude_u_add_offset)
# print(np.max(longitude_u_raw))
# print(np.min(longitude_u_raw))

# longitude_v_scale_factor = np.array(rootgrp.variables["longitude_v"].scale_factor)
# longitude_v_add_offset = np.array(rootgrp.variables["longitude_v"].add_offset)

# latitude_v_scale_factor = np.array(rootgrp.variables["latitude_v"].scale_factor)
# latitude_v_add_offset = np.array(rootgrp.variables["latitude_v"].add_offset)

# longitude_v = np.array(rootgrp.variables["longitude_v"])*longitude_v_scale_factor + longitude_v_add_offset
# latitude_v = np.array(rootgrp.variables["latitude_v"])*latitude_v_scale_factor + latitude_v_add_offset



rootgrp.close()

# plt.plot(longitude_u.flatten(), latitude_u.flatten(), '.')
# plt.plot(longitude_v.flatten(), latitude_v.flatten(), '.', color='red')

# Lat & Long are in degree

inProj = Proj(init='epsg:4326')
outProj = Proj(init='epsg:2154')

# i_min = 128  
# i_max = 144
# j_min = 494
# j_max = 496

# j_min = 127  
# j_max = 152
# i_min = 494
# i_max = 501

j_min = 127  
j_max = 148
i_min = 494
i_max = 501

# 127 152
# 494 501

# i_min = 0
# i_max = -1
# j_min = 0
# j_max = -1


# print("latitude = ", latitude_u[i][j], " longitude = ", longitude_u[i][j])
x_u,y_u = transform(inProj,outProj,longitude_u, latitude_u)

print(x_u[0][0], y_u[0][0])

# print("x = ", x_u[i][j], " y = ", y_u[i][j])

plt.scatter((x_u[i_min:i_max,j_min:j_max]).flatten(), (y_u[i_min:i_max,j_min:j_max]).flatten(), c=(u[i_min:i_max,j_min:j_max]).flatten())

x_search = [95000, 102490, 102490, 95000, 95000]
y_search = [6848000, 6848000, 6849100, 6849100, 6848000]
plt.plot(x_search, y_search, 'b')

x_search = [95000, 100000, 100000, 95000, 95000]
y_search = [6848000, 6848000, 6849100, 6849100, 6848000]
plt.plot(x_search, y_search, 'r')


# ## Ouessant

lon_ouessant = np.array([-5.243568420410157,-5.241508483886719,-4.967536926269532,-4.979896545410157,-5.243568420410157])
lat_ouessant = np.array([48.494995030818075,48.40481902832667,48.4043631875212,48.51046369041123,48.494995030818075])

x_ouessant, y_ouessant = transform(inProj,outProj,lon_ouessant, lat_ouessant)

plt.plot(x_ouessant, y_ouessant, color='red')

plt.show()