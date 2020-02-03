# font-slicer

font-slicer renders freely scalable font glyphs using the GPU.

It works by decomposing glyph outlines into monotone trapezoids, and
calculating the coverage for each trapezoid with a pixel shader.  The files
`font_slicer.h` and `font_slicer.cpp` contain the core code which decomposes
glyph outlines.


## Usage

The example program runs on OSX.  An Xcode project is provided.  To compile,
you will need to link to [FreeType](http://freetype.org).

To run the program provide a font file as an argument:

    font-slicer myfont.ttf


## Algorithm

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

The text rendered by font-slicer is freely scalable, but cannot be rotated.  I
present it here as a potential solution for rasterising text on the GPU.


## License

Copyright © 2014 Edmund Kapusniak. Licensed under the GNU General Public
License, version 3. See the LICENSE file in the project root for full license
information.

