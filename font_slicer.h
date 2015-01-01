//
//  font_slicer.h
//
//  Created by Edmund Kapusniak on 22/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef FONT_SLICER_H
#define FONT_SLICER_H


#include <vector>
#include <memory>
#include <bezier.h>
#include <rect.h>


/*
    Loads a font and approximates each glyph with a set of slices.  Each slice
    is a trapezoid where the left and right edges are quadratic bezier curves.

    
        -------/-----'-.--------------  maxy
              /            .
             /                .
            /                   .
        ---/---------------------`----  miny
                                  '
        left                   right


    Font metrics:

        ascender    : distance from baseline to top of font, in pixels.
        descender   : distance from baseline to bottom of font, in pixels.
        line_height : distance between consecutive lines.
        
    Y is up.  Note that the descender is negative when below the baseline.

*/


struct font_slice
{
    qbezier left;
    qbezier right;
};


struct font_glyph
{
    char32_t    c;
    float       advance;
    rect        bounds;
    std::vector< font_slice > slices;
};


struct font_kern
{
    char32_t    a;
    char32_t    b;
    float       kerning;
};


class font_slicer
{
public:

    explicit font_slicer( const char* path );
    ~font_slicer();
    
    float units_per_em();

    float ascender();
    float descender();
    float line_height();

    size_t glyph_count();
    char32_t glyph_char( size_t index );
    font_glyph glyph_info( size_t index );
    font_glyph glyph_info_for_char( char32_t c );
    
    size_t kern_count();
    font_kern kern( size_t index );


private:

    struct impl;
    std::unique_ptr< impl > p;

};


#endif
