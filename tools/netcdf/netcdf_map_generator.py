from netCDF4 import Dataset
import numpy as np
import vtk

directory = "/home/lemezoth/Documents/ensta/flotteur/data_ifremer/data/"
filename = "MARC_L1-MARS2D-FINIS250_20170709T0000Z_MeteoMF.nc"

dataset = Dataset(directory + filename)

u = dataset.variables['U']
fillValue = u._FillValue
shape = u.shape

print(u[0][0][0])
if(u[0][0][0] is np.ma.masked):
	print("test OK")
print(shape)

polydata = vtk.vtkAppendPolyData()

for j in range(u.shape[1]):
	print(j)
	for i in range(u.shape[2]):
		if(u[0][j][i] is np.ma.masked):
			cube = vtk.vtkCubeSource()
			cube.SetBounds(0.0, 15.0*60.0, j*250.0, (j+1)*250.0, i*250.0, (i+1)*250.0)
			cube.Update()

			polydata.AddInputData(cube.GetOutput())
polydata.Update()

outputWriter = vtk.vtkXMLPolyDataWriter()
outputWriter.SetFileName("map.vtp")
outputWriter.SetInputData(polydata.GetOutput());
outputWriter.Write()