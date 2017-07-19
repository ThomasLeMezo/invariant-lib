#!/usr/bin/python3.4

#### import the simple module from the paraview
from paraview.simple import *

renderView1 = GetActiveViewOrCreate('RenderView')
renderView1.AxesGrid.Visibility = 1

for key, value in GetSources().items():
	# Case output.vtp
	if(key[0][-16:]=='doors_output.vtp'):
		display = GetDisplayProperties(value, view=renderView1)
		display.DiffuseColor = [0.0, 0.0, 1.0] # BLUE

	if(key[0][-15:]=='doors_input.vtp'):
		display = GetDisplayProperties(value, view=renderView1)
		display.DiffuseColor = [1.0, 0.0, 0.0] # RED

	if(key[0][-9:]=='paves.vtp'):
		display = GetDisplayProperties(value, view=renderView1)
		display.SetRepresentationType('Wireframe')

	if(key[0][-16:]=='vector_field.vtp'):
		# display = GetDisplayProperties(value, view=renderView1)
		Hide(value, renderView1)
		glyph1 = Glyph(Input=value,GlyphType='Arrow')
		glyph1.Scalars = ['POINTS', 'None']
		glyph1.Vectors = ['POINTS', 'Glyph']
		glyph1.ScaleFactor = 1000.0
		glyph1.GlyphTransform = 'Transform2'
		glyph1.ScaleMode = 'scalar'
		glyph1.GlyphMode = 'All Points'

		# show data in view
		glyph1Display = Show(glyph1, renderView1)
		# trace defaults for the display properties.
		glyph1Display.ColorArrayName = [None, '']
		glyph1Display.GlyphType = 'Arrow'
