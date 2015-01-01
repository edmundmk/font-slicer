font-slicer
===========

font-slicer renders freely scalable font glyphs using the GPU.

It takes an approach similar to CPU rasterisation, by 'slicing' each glyph's
outline into y-monotone pieces.  Each slice is trapezoidal - it has a
horizontal top and bottom edge, and left and right edges which are quadratic
Bézier curves.

Each slice is rendered by the GPU as a quad.  The vertex shader snaps the quad
to pixel boundaries to ensure that each pixel potentially covered by the slice
is considered.

The pixel shader calculates an approximation of the exact coverage of the slice
on the pixel.  The pixel is treated as a square, and the slice is clipped to
it.  The curved edges of the slice are flattened to lines, to produce a true
trapezoid, and the percentage coverage of the trapezoid on the square is
output from the shader.

OpenGL's built-in sRGB handling performs gamma correction on the text for
display.

The text rendered by font-slicer is freely scalable, but cannot be rotated.  I
present it here as a potential solution for rasterising text on the GPU, as an
an alternative to texture atlases,
[SDFs](http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf),
[glyphy](http://glyphy.org), or
[Loop-Blinn](http://http.developer.nvidia.com/GPUGems3/gpugems3_ch25.html).

To compile, you will need to link to [FreeType](http://freetype.org).

_Edmund Kapusniak_

