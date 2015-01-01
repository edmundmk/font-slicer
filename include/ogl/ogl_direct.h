//
//  ogl_direct.h
//
//  Created by Edmund Kapusniak on 01/01/2015.
//  Copyright (c) 2015 Edmund Kapusniak. All rights reserved.
//


#ifndef OGL_DIRECT_H
#define OGL_DIRECT_H


#include <math3.h>
#include "ogl_headers.h"


class ogl_context;



/*
    glBegin-style direct vertex specification.  Allows experimentation.
    Attribute indexes are:
    
        float4 position : 0
        float4 colour   : 1
        float4 texcoord : 2
 
*/


class ogl_direct
{
public:

    explicit ogl_direct( ogl_context* ogl );
    ~ogl_direct();
    
    
    void begin( GLenum pmode );
    void colour( float r, float g, float b, float a = 1.0 );
    void colour( float4 colour );
    void texcoord( float x, float y, float z = 0.0f, float w = 1.0f );
    void texcoord( float4 texcoord );
    void vertex( float x, float y, float z = 0.0f, float w = 1.0f );
    void vertex( float4 position );
    void end();
    
    
private:

    struct v
    {
        float4 position;
        float4 colour;
        float4 texcoord;
    };
    
    
    void submit();
    
    
    ogl_context* ogl;

    GLuint  vao;
    GLuint  vbo;
    size_t  offset;

    float4  vcolour;
    float4  vtexcoord;

    GLenum  mode    : 16;
    bool    isfirst : 1;
    v       first;
    v       prev1;
    v       prev0;
    v*      p;
    size_t  index;
    size_t  count;
    

};



inline void ogl_direct::colour( float r, float g, float b, float a )
{
    colour( float4( r, g, b, a ) );
}

inline void ogl_direct::colour( float4 colour )
{
    vcolour = colour;
}

inline void ogl_direct::texcoord( float x, float y, float z, float w )
{
    texcoord( float4( x, y, z, w ) );
}

inline void ogl_direct::texcoord( float4 texcoord )
{
    vtexcoord = texcoord;
}

inline void ogl_direct::vertex( float x, float y, float z, float w )
{
    vertex( float4( x, y, z, w ) );
}

inline void ogl_direct::vertex( float4 position )
{
    v vertex;
    vertex.position = position;
    vertex.colour   = vcolour;
    vertex.texcoord = vtexcoord;
    
    if ( isfirst )
    {
        first = vertex;
        isfirst = false;
    }
    
    prev1 = prev0;
    prev0 = vertex;
    
    if ( index >= count )
    {
        submit();
    }
    
    p[ index++ ] = vertex;
}



#endif
