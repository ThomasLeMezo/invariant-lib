#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

renderView1 = GetActiveViewOrCreate('RenderView')

# create a new 'Box'
boxA = Box()
RenameSource('Box_A', boxA)
boxA.XLength = 300.0
boxA.YLength = 2000.0
boxA.ZLength = 2000.0
boxA.Center = [150.0, 34250.0, 119250.0]
boxADisplay = Show(boxA, renderView1)
boxADisplay.SetRepresentationType('Wireframe')
boxADisplay.AmbientColor = [0.0, 1.0, 0.0]
SetActiveSource(boxA)

boxB = Box()
RenameSource('Box_B', boxB)
boxB.XLength = 300.0
boxB.YLength = 2000.0
boxB.ZLength = 2000.0
boxB.Center = [37650.0, 34250.0, 119250.0]
boxBDisplay = Show(boxB, renderView1)
boxBDisplay.SetRepresentationType('Wireframe')
boxBDisplay.AmbientColor = [1.0, 0.6666666666666666, 0.0]
SetActiveSource(boxB)

boxC = Box()
RenameSource('Box_C', boxC)
boxC.XLength = 10000.0
boxC.YLength = 3000.0
boxC.ZLength = 3000.0
boxC.Center = [23000.0, 46100.0, 125000.0]
boxCDisplay = Show(boxC, renderView1)
boxCDisplay.SetRepresentationType('Wireframe')
boxCDisplay.AmbientColor = [1.0, 0.0, 0.0]
SetActiveSource(boxC)

# create a new 'XML PolyData Reader'
mapvtp = XMLPolyDataReader(FileName=['/home/lemezoth/Desktop/map.vtp'])
RenameSource('Map', mapvtp)
# show data in view
mapvtpDisplay = Show(mapvtp, renderView1)
mapvtpDisplay.DiffuseColor = [0.3333333333333333, 0.0, 0.0]

# update the view to ensure updated data information
renderView1.Update()
