//
//  ogl_direct.cpp
//
//  Created by Edmund Kapusniak on 01/01/2015.
//  Copyright (c) 2015 Edmund Kapusniak. All rights reserved.
//


#include "ogl_direct.h"
#include "ogl_context.h"



/*
    Allow an unbounded number of vertices while submitting primitives in
    batches.  Requires remembering enough information to allow restart.
    
    GL_POINTS           easy
    GL_LINE_STRIP       remember last vertex
    GL_LINE_LOOP        remember first and last vertex
    GL_LINES            submit in batches divisible by 2
    GL_TRIANGLE_STRIP   remember previous 2 vertices, submit divisible by 2
    GL_TRIANGLE_FAN     remember first and last vertex
    GL_TRIANGLES        submit in batches divisible by 3

*/


static const size_t VERTEX_COUNT = 341;
static const size_t MIN_VERTICES = 12;



ogl_direct::ogl_direct( ogl_context* ogl )
    :   ogl( ogl )
    ,   vao( 0 )
    ,   vbo( 0 )
    ,   offset( 0 )
    ,   vcolour( 0.0f, 0.0f, 0.0f, 1.0f )
    ,   vtexcoord( 0.0f, 0.0f, 0.0f, 1.0f )
    ,   mode( GL_NONE )
    ,   isfirst( false )
    ,   p( nullptr )
    ,   index( 0 )
    ,   count( 0 )
{
    ogl->glGenVertexArrays( 1, &vao );
    ogl->glGenBuffers( 1, &vbo );
    
    ogl->glBindBuffer( GL_ARRAY_BUFFER, vbo );
    ogl->glBufferData( GL_ARRAY_BUFFER,
                sizeof( v ) * VERTEX_COUNT, nullptr, GL_STREAM_DRAW );
    
    ogl->glBindVertexArray( vao );
    ogl->glEnableVertexAttribArray( 0 );
    ogl->glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE,
                sizeof( v ), (const GLvoid*)offsetof( v, position ) );
    ogl->glEnableVertexAttribArray( 1 );
    ogl->glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE,
                sizeof( v ), (const GLvoid*)offsetof( v, colour ) );
    ogl->glEnableVertexAttribArray( 2 );
    ogl->glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE,
                sizeof( v ), (const GLvoid*)offsetof( v, texcoord ) );
    ogl->glBindVertexArray( 0 );
    
    ogl->glBindBuffer( GL_ARRAY_BUFFER, 0 );
    

}

ogl_direct::~ogl_direct()
{
    ogl->glDeleteBuffers( 1, &vbo );
    ogl->glDeleteVertexArrays( 1, &vao );
}



void ogl_direct::begin( GLenum pmode )
{
    assert( mode == GL_NONE );
    assert( ogl->EXT_map_buffer_range );
    
    // Begin drawing.
    mode = pmode;
    isfirst = ( mode == GL_LINE_LOOP || mode == GL_TRIANGLE_FAN );

    // Perform buffer object streaming using an unsynchronised mapping.
    ogl->glBindVertexArray( vao );
    ogl->glBindBuffer( GL_ARRAY_BUFFER, vbo );

    // See how much space is left in this buffer.
    if ( VERTEX_COUNT - offset < MIN_VERTICES )
    {
        // Orphan existing buffer (as it may still be used by GL).
        ogl->glBufferData( GL_ARRAY_BUFFER,
                    sizeof( v ) * VERTEX_COUNT, nullptr, GL_STREAM_DRAW );
        offset = 0;
    }
    
    // Map remaining section of buffer (we don't know how big this batch is).

    // GL_MAP_INVALIDATE_RANGE_BIT is pretty much mandatory if we want to avoid
    // forcing the driver to give us back the same memory - even if we never
    // read the driver doesn't know which particular bytes we write, so if we
    // don't invalidate then those non-written bytes must be initialized with
    // the existing buffer contents.

    count = VERTEX_COUNT - offset;
    if ( mode == GL_LINES || mode == GL_TRIANGLE_STRIP )
        count = ( count / 2 ) * 2;
    else if ( mode == GL_TRIANGLES )
        count = ( count / 3 ) * 3;
    assert( count >= MIN_VERTICES );
    
    p = (v*)ogl->glMapBufferRange
    (
        GL_ARRAY_BUFFER,
        sizeof( v ) * offset,
        sizeof( v ) * count,
        GL_MAP_WRITE_BIT
            | GL_MAP_INVALIDATE_RANGE_BIT
            | GL_MAP_FLUSH_EXPLICIT_BIT
            | GL_MAP_UNSYNCHRONIZED_BIT
    );
    index = 0;

}

void ogl_direct::submit()
{
    assert( mode != GL_NONE );
    assert( index == count );
    
    // Assume vao and buffer are still bound.

    // Flush edited part of buffer (now we know how much data was written).
    ogl->glFlushMappedBufferRange
    (
        GL_ARRAY_BUFFER,
        sizeof( v ) * offset,
        sizeof( v ) * index
    );
    ogl->glUnmapBuffer( GL_ARRAY_BUFFER );

    // Buffer is full.  Draw what we have.
    assert( index >= MIN_VERTICES );
    GLenum drawmode = ( mode == GL_LINE_LOOP ) ? GL_LINE_STRIP : mode;
    ogl->glDrawArrays( drawmode, (GLint)offset, (GLsizei)count );
    
    // Orphan the buffer.
    ogl->glBufferData( GL_ARRAY_BUFFER,
                sizeof( v ) * VERTEX_COUNT, nullptr, GL_STREAM_DRAW );
    offset = 0;
    
    // Remap buffer.
    count = VERTEX_COUNT - offset;
    if ( mode == GL_LINES || mode == GL_TRIANGLE_STRIP )
        count = ( count / 2 ) * 2;
    else if ( mode == GL_TRIANGLES )
        count = ( count / 3 ) * 3;
    assert( count >= MIN_VERTICES );

    p = (v*)ogl->glMapBufferRange
    (
        GL_ARRAY_BUFFER,
        sizeof( v ) * offset,
        sizeof( v ) * count,
        GL_MAP_WRITE_BIT
            | GL_MAP_INVALIDATE_RANGE_BIT
            | GL_MAP_FLUSH_EXPLICIT_BIT
            | GL_MAP_UNSYNCHRONIZED_BIT
    );
    index = 0;
    
    // Restart primitive.
    switch ( mode )
    {
    case GL_POINTS:
    case GL_LINES:
    case GL_TRIANGLES:
        // Batch should have been a whole number of primitives.
        break;
        
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
        // Restart from previous vertex.
        p[ index++ ] = prev0;
        break;
        
    case GL_TRIANGLE_STRIP:
        // Restart using previous two vertices.  The fact that the batch
        // contained an even number of vertices should mean that the resulting
        // triangles have the same winding order.
        p[ index++ ] = prev1;
        p[ index++ ] = prev0;
        break;
        
    case GL_TRIANGLE_FAN:
        // Restart with the first vertex, and the last vertex from the fan.
        p[ index++ ] = first;
        p[ index++ ] = prev0;
        break;
    }
    
    assert( index <= count );
}

void ogl_direct::end()
{
    assert( mode != GL_NONE );
    assert( index <= count );
    
    // Check if we've drawn any vertices at all.
    if ( index == 0 )
    {
        return;
    }
    
    // If we're drawing a line loop then draw back to the first vertex.
    if ( mode == GL_LINE_LOOP )
    {
        if ( index >= count )
        {
            submit();
        }
        
        p[ index++ ] = first;
    }
    
    // Assume vao and buffer are still bound.
    
    // Flush edited part of buffer (now we know how much data was written).
    ogl->glFlushMappedBufferRange
    (
        GL_ARRAY_BUFFER,
        sizeof( v ) * offset,
        sizeof( v ) * index
    );
    ogl->glUnmapBuffer( GL_ARRAY_BUFFER );
    
    // Draw.
    GLenum drawmode = ( mode == GL_LINE_LOOP ) ? GL_LINE_STRIP : mode;
    ogl->glDrawArrays( drawmode, (GLint)offset, (GLsizei)index );

    // Unbind.
    ogl->glBindBuffer( GL_ARRAY_BUFFER, 0 );
    ogl->glBindVertexArray( 0 );
    
    // Next buffer write should happen after this one.
    offset += index;
    
    // Stop drawing.
    mode    = GL_NONE;
    p       = nullptr;
    index   = 0;
    count   = 0;
    
}






