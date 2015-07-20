//
//  main.cpp
//
//  Created by Edmund Kapusniak on 18/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <make_unique.h>
#include <strpath.h>
#include <math3.h>
#include <uic/uic_application.h>
#include <uic/uic_window.h>
#include <uic/uic_glcanvas.h>
#include <ogl/ogl_context.h>

#include "font_slicer.h"



static const char* jabberwocky =
"`Twas brillig, and the slithy toves\n"
"  Did gyre and gimble in the wabe:\n"
"All mimsy were the borogoves,\n"
"  And the mome raths outgrabe.\n"
"\n"
"\"Beware the Jabberwock, my son!\n"
"  The jaws that bite, the claws that catch!\n"
"Beware the Jubjub bird, and shun\n"
"  The frumious Bandersnatch!\"\n"
"He took his vorpal sword in hand:\n"
"  Long time the manxome foe he sought --\n"
"So rested he by the Tumtum tree,\n"
"  And stood awhile in thought.\n"
"And, as in uffish thought he stood,\n"
"  The Jabberwock, with eyes of flame,\n"
"Came whiffling through the tulgey wood,\n"
"  And burbled as it came!\n"
"One, two! One, two! And through and through\n"
"  The vorpal blade went snicker-snack!\n"
"He left it dead, and with its head\n"
"  He went galumphing back.\n"
"\"And, has thou slain the Jabberwock?\n"
"  Come to my arms, my beamish boy!\n"
"O frabjous day! Callooh! Callay!'\n"
"  He chortled in his joy.\n"
"\n"
"`Twas brillig, and the slithy toves\n"
"  Did gyre and gimble in the wabe;\n"
"All mimsy were the borogoves,\n"
"  And the mome raths outgrabe.\n"
;




static const char* vertex_shader =
"uniform mat3 u_transform;\n"
"uniform vec2 u_viewport;\n"
"\n"
"attribute vec2 a_position;\n"
"attribute vec2 a_rounding;\n"
"attribute vec2 a_l0;\n"
"attribute vec2 a_l1;\n"
"attribute vec2 a_l2;\n"
"attribute vec2 a_r0;\n"
"attribute vec2 a_r1;\n"
"attribute vec2 a_r2;\n"
"\n"
"varying vec2 v_l0;\n"
"varying vec2 v_l1;\n"
"varying vec2 v_l2;\n"
"varying vec2 v_r0;\n"
"varying vec2 v_r1;\n"
"varying vec2 v_r2;\n"
"\n"
"void main()\n"
"{\n"
"    // Transform into viewport coordinates.\n"
"    v_l0 = ( vec3( a_l0, 1.0 ) * u_transform ).xy;\n"
"    v_l1 = ( vec3( a_l1, 1.0 ) * u_transform ).xy;\n"
"    v_l2 = ( vec3( a_l2, 1.0 ) * u_transform ).xy;\n"
"    v_r0 = ( vec3( a_r0, 1.0 ) * u_transform ).xy;\n"
"    v_r1 = ( vec3( a_r1, 1.0 ) * u_transform ).xy;\n"
"    v_r2 = ( vec3( a_r2, 1.0 ) * u_transform ).xy;\n"
"\n"
"    // Round quad to pixel border.\n"
"    vec2 p = ( vec3( a_position, 1.0 ) * u_transform ).xy;\n"
"    p = floor( p + a_rounding );\n"
"\n"
"    // And transform from viewport to clip.\n"
"    p = p * ( 2.0 / u_viewport ) - vec2( 1.0, 1.0 );\n"
"    gl_Position = vec4( p, 0.0, 1.0 );\n"
"}\n"
;

static const char* fragment_shader =
"varying vec2 v_l0;\n"
"varying vec2 v_l1;\n"
"varying vec2 v_l2;\n"
"varying vec2 v_r0;\n"
"varying vec2 v_r1;\n"
"varying vec2 v_r2;\n"
"\n"
"const float EPSILON = 1.0e-4;"
"\n"
"float solve( vec2 p0, vec2 p1, vec2 p2, float y )\n"
"{\n"
"    float a = p0.y - 2.0 * p1.y + p2.y;\n"
"    float b = -2.0 * p0.y + 2.0 * p1.y;\n"
"    float c = p0.y - y;\n"
"    float d = b * b - 4.0 * a * c;\n"
"\n"
"    float t;\n"
"    if ( abs( a ) > EPSILON )\n"
"    {\n"
"        d = sqrt( max( d, 0.0 ) );"
"        float t0 = ( -b - d ) / ( 2.0 * a );\n"
"        float t1 = ( -b + d ) / ( 2.0 * a );\n"
"        if ( t0 >= 0.0 && t0 <= 1.0 )\n"
"            t = t0;\n"
"        else\n"
"            t = t1;\n"
"    }\n"
"    else\n"
"    {\n"
"        t = -c / b;\n"
"    }\n"
"\n"
"    return mix( mix( p0.x, p1.x, t ), mix( p1.x, p2.x, t ), t );\n"
"}\n"
"\n"
"float xcoverage( float l, float r, float minl, float maxl, float minr, float maxr )\n"
"{\n"
"    /*\n"
"         Work out clipped trapeziod wedge.\n"
"\n"
"                    maxl ______ minr                   \n"
"                       . .    .  .                     \n"
"                     .   .    .     .                  \n"
"                   .|    .    .    |   .               \n"
"                 .  |a   .b   .c   |d     .            \n"
"               .    |    .    .    |         .         \n"
"             .      |____.____.____|            .      \n"
"          minl         m    n    o              maxr   \n"
"    */\n"
"    float a = mix( 0.0, 1.0, clamp( ( l - minl ) / ( maxl - minl ), 0.0, 1.0 ) );\n"
"    float b = mix( 0.0, 1.0, clamp( ( r - minl ) / ( maxl - minl ), 0.0, 1.0 ) );\n"
"    float c = mix( 1.0, 0.0, clamp( ( l - minr ) / ( maxr - minr ), 0.0, 1.0 ) );\n"
"    float d = mix( 1.0, 0.0, clamp( ( r - minr ) / ( maxr - minr ), 0.0, 1.0 ) );\n"
"\n"
"    float m = max( min( maxl, r ) - max( minl, l ), 0.0 );\n"
"    float n = max( min( minr, r ) - max( maxl, l ), 0.0 );\n"
"    float o = max( min( maxr, r ) - max( minr, l ), 0.0 );\n"
"\n"
"    return ( a + b ) * 0.5 * m + n + ( c + d ) * 0.5 * o;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    // Work out vertical coverage of the slice on this pixel.\n"
"    float miny = max( v_l0.y, gl_FragCoord.y - 0.5 );\n"
"    float maxy = min( v_l2.y, gl_FragCoord.y + 0.5 );\n"
"    float ycoverage = max( 0.0, maxy - miny );\n"
"\n"
"    // Solve to find corners of trapezoid.\n"
"    float tl = solve( v_l0, v_l1, v_l2, miny );\n"
"    float bl = solve( v_l0, v_l1, v_l2, maxy );\n"
"    float tr = solve( v_r0, v_r1, v_r2, miny );\n"
"    float br = solve( v_r0, v_r1, v_r2, maxy );\n"
"\n"
"    float minl = min( tl, bl );\n"
"    float maxl = max( tl, bl );\n"
"    float minr = min( tr, br );\n"
"    float maxr = max( tr, br );\n"
"\n"
"    float l = gl_FragCoord.x - 0.5;\n"
"    float r = gl_FragCoord.x + 0.5;\n"
"\n"
"    float coverage = xcoverage( l, r, minl, maxl, minr, maxr ) * ycoverage;\n"
"    gl_FragColor = vec4( coverage, coverage, coverage, 1.0 );\n"
"}\n"
;


static const char* blit_vshader =
"attribute vec2 a_position;\n"
"attribute vec2 a_texcoord;\n"
"\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main()\n"
"{\n"
"    v_texcoord = a_texcoord;\n"
"    gl_Position = vec4( a_position, 0.0, 1.0 );\n"
"}\n"
;

static const char* blit_fshader =
"uniform sampler2D u_texture;\n"
"\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 p = texture2D( u_texture, v_texcoord );\n"
"\n"
"    vec4 bg = vec4( 1.0, 1.0, 1.0, 1.0 );\n"
"    vec4 fg = vec4( 0.0, 0.0, 0.0, 1.0 );\n"
"    gl_FragColor = mix( bg, fg, p );\n"
"}\n"
;



struct vertex
{
    float2  position;
    float2  rounding;
    float2  l0;
    float2  l1;
    float2  l2;
    float2  r0;
    float2  r1;
    float2  r2;
};


struct blit_vertex
{
    float2  position;
    float2  texcoord;
};


struct glyph
{
    float advance;
    GLsizei count;
    const GLvoid* indices;
};


class fe_glcanvas : public uic_glcanvas
{
public:

    explicit fe_glcanvas( const char* font_path );


protected:

    virtual void setup_context( ogl_context* ogl );
    virtual void draw( ogl_context* ogl );

    virtual void on_mouse_down( float2 p, uic_button button );
    virtual void on_mouse_up( float2 p, uic_button button );
    virtual void on_mouse_move( float2 p );
    virtual void on_scroll( float2 p, float amount );
    virtual void on_zoom( float2 p, float amount );


private:

    std::string font_path;

    GLuint program;
    GLint u_transform;
    GLint u_viewport;

    GLuint blit;
    GLint u_texture;

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    
    GLuint fbo;
    GLsizei texture_width;
    GLsizei texture_height;
    GLuint texture;
    
    GLuint blit_vao;
    GLuint blit_vbo;
    GLuint blit_ibo;
    
    float emsize;
    float line_height;
    std::unordered_map< char32_t, glyph > glyphs;
    std::unordered_map< uint64_t, float > kerning;

    float2 offset;
    float  scale;
    
    bool drag_active;
    float2 drag_start;

};










fe_glcanvas::fe_glcanvas( const char* font_path )
    :   font_path( font_path )
    ,   program( 0 )
    ,   u_transform( -1 )
    ,   u_viewport( -1 )
    ,   vao( 0 )
    ,   vbo( 0 )
    ,   ibo( 0 )
    ,   emsize( 0.0f )
    ,   offset( 200.0f, 600.0f )
    ,   scale( 5.0f )
    ,   drag_active( false )
    ,   drag_start( 0.0f, 0.0f )
{
}



void fe_glcanvas::setup_context( ogl_context* ogl )
{
    GLuint vshader = ogl->compile_shader( GL_VERTEX_SHADER, vertex_shader );
    GLuint fshader = ogl->compile_shader( GL_FRAGMENT_SHADER, fragment_shader );
    program = ogl->glCreateProgram();
    ogl->glAttachShader( program, vshader );
    ogl->glAttachShader( program, fshader );
    ogl->glDeleteShader( vshader );
    ogl->glDeleteShader( fshader );
    ogl->glBindAttribLocation( program, 0, "a_position" );
    ogl->glBindAttribLocation( program, 1, "a_rounding" );
    ogl->glBindAttribLocation( program, 2, "a_l0" );
    ogl->glBindAttribLocation( program, 3, "a_l1" );
    ogl->glBindAttribLocation( program, 4, "a_l2" );
    ogl->glBindAttribLocation( program, 5, "a_r0" );
    ogl->glBindAttribLocation( program, 6, "a_r1" );
    ogl->glBindAttribLocation( program, 7, "a_r2" );
    ogl->link_program( program );
    
    u_transform = ogl->glGetUniformLocation( program, "u_transform" );
    u_viewport = ogl->glGetUniformLocation( program, "u_viewport" );
    
    
    vshader = ogl->compile_shader( GL_VERTEX_SHADER, blit_vshader );
    fshader = ogl->compile_shader( GL_FRAGMENT_SHADER, blit_fshader );
    blit = ogl->glCreateProgram();
    ogl->glAttachShader( blit, vshader );
    ogl->glAttachShader( blit, fshader );
    ogl->glDeleteShader( vshader );
    ogl->glDeleteShader( fshader );
    ogl->glBindAttribLocation( blit, 0, "a_position" );
    ogl->glBindAttribLocation( blit, 1, "a_texcoord" );
    ogl->link_program( blit );
    
    u_texture = ogl->glGetUniformLocation( blit, "u_texture" );
    
    ogl->glUseProgram( blit );
    ogl->glUniform1i( u_texture, 0 );
    ogl->glUseProgram( 0 );
    
    
    std::vector< vertex > vbuffer;
    std::vector< GLuint > ibuffer;
    
    font_slicer fs( font_path.c_str() );
    emsize = fs.units_per_em();
    line_height = fs.line_height();
    
    for ( const char* j = jabberwocky; *j; ++j )
    {
        char32_t c = *j;
        if ( glyphs.find( c ) != glyphs.end() )
            continue;
    
        font_glyph g = fs.glyph_info_for_char( c );

        glyph gc;
        gc.advance = g.advance;
        gc.count = 0;
        gc.indices = (const GLvoid*)( ibuffer.size() * sizeof( GLuint ) );
        
        for ( size_t i = 0; i < g.slices.size(); ++i )
        {
            const font_slice& s = g.slices[ i ];
            
            vertex v;
            v.l0 = s.left.p[ 0 ];
            v.l1 = s.left.p[ 1 ];
            v.l2 = s.left.p[ 2 ];
            v.r0 = s.right.p[ 0 ];
            v.r1 = s.right.p[ 1 ];
            v.r2 = s.right.p[ 2 ];
            
            rect r
            (
                min( min( v.l0.x, v.l1.x ), v.l2.x ),
                v.l0.y,
                max( max( v.r0.x, v.r1.x ), v.r2.x ),
                v.l2.y
            );


            /*
                2    3
            
                0    1
            */
            
            GLuint base = (GLuint)vbuffer.size();
            ibuffer.emplace_back( base + 0 );
            ibuffer.emplace_back( base + 1 );
            ibuffer.emplace_back( base + 2 );

            ibuffer.emplace_back( base + 2 );
            ibuffer.emplace_back( base + 1 );
            ibuffer.emplace_back( base + 3 );
            
            v.position = float2( r.minx, r.miny );
            v.rounding = float2( 0.0f, 0.0f );
            vbuffer.push_back( v );

            v.position = float2( r.maxx, r.miny );
            v.rounding = float2( 1.0f, 0.0f );
            vbuffer.push_back( v );
            
            v.position = float2( r.minx, r.maxy );
            v.rounding = float2( 0.0f, 1.0f );
            vbuffer.push_back( v );
            
            v.position = float2( r.maxx, r.maxy );
            v.rounding = float2( 1.0f, 1.0f );
            vbuffer.push_back( v );
            
            gc.count += 6;
        }
        
        glyphs.emplace( g.c, gc );
    }

    
    for ( size_t i = 0; i < fs.kern_count(); ++i )
    {
        font_kern k = fs.kern( i );
        uint64_t key = (uint64_t)k.a << 32 | (uint64_t)k.b;
        kerning.emplace( key, k.kerning );
    }
    
    
    ogl->glGenBuffers( 1, &vbo );
    ogl->glBindBuffer( GL_ARRAY_BUFFER, vbo );
    ogl->glBufferData( GL_ARRAY_BUFFER, vbuffer.size() * sizeof( vertex ), vbuffer.data(), GL_STATIC_DRAW );

    ogl->glGenBuffers( 1, &ibo );
    ogl->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    ogl->glBufferData( GL_ELEMENT_ARRAY_BUFFER, ibuffer.size() * sizeof( GLuint ), ibuffer.data(), GL_STATIC_DRAW );
    
    ogl->glGenVertexArrays( 1, &vao );
    ogl->glBindVertexArray( vao );
    ogl->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    ogl->glEnableVertexAttribArray( 0 );
    ogl->glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, position ) );
    ogl->glEnableVertexAttribArray( 1 );
    ogl->glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, rounding ) );
    ogl->glEnableVertexAttribArray( 2 );
    ogl->glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, l0 ) );
    ogl->glEnableVertexAttribArray( 3 );
    ogl->glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, l1 ) );
    ogl->glEnableVertexAttribArray( 4 );
    ogl->glVertexAttribPointer( 4, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, l2 ) );
    ogl->glEnableVertexAttribArray( 5 );
    ogl->glVertexAttribPointer( 5, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, r0 ) );
    ogl->glEnableVertexAttribArray( 6 );
    ogl->glVertexAttribPointer( 6, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, r1 ) );
    ogl->glEnableVertexAttribArray( 7 );
    ogl->glVertexAttribPointer( 7, 2, GL_FLOAT, GL_FALSE, sizeof( vertex ), (const GLvoid*)offsetof( vertex, r2 ) );

    ogl->glBindVertexArray( 0 );
    ogl->glBindBuffer( GL_ARRAY_BUFFER, 0 );
    ogl->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    texture_width = 1920;
    texture_height = 1080;
    ogl->glGenTextures( 1, &texture );
    ogl->glBindTexture( GL_TEXTURE_2D, texture );
    ogl->glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    ogl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    ogl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    ogl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    ogl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    ogl->glBindTexture( GL_TEXTURE_2D, 0 );

    ogl->glGenFramebuffers( 1, &fbo );
    ogl->glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    ogl->glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    ogl->glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    ogl->glGenBuffers( 1, &blit_vbo );
    ogl->glBindBuffer( GL_ARRAY_BUFFER, blit_vbo );
    ogl->glBufferData( GL_ARRAY_BUFFER, sizeof( blit_vertex ) * 4, NULL, GL_STREAM_DRAW );
    
    static const GLubyte indices[] = { 0, 1, 2, 2, 1, 3 };
    
    ogl->glGenBuffers( 1, &blit_ibo );
    ogl->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, blit_ibo );
    ogl->glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( GLubyte ) * 6, indices, GL_STATIC_DRAW );
    
    ogl->glGenVertexArrays( 1, &blit_vao );
    ogl->glBindVertexArray( blit_vao );
    ogl->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, blit_ibo );
    ogl->glEnableVertexAttribArray( 0 );
    ogl->glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( blit_vertex ), (const GLvoid*)offsetof( blit_vertex, position ) );
    ogl->glEnableVertexAttribArray( 1 );
    ogl->glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( blit_vertex ), (const GLvoid*)offsetof( blit_vertex, texcoord ) );
    
    ogl->glBindVertexArray( 0 );
    ogl->glBindBuffer( GL_ARRAY_BUFFER, 0 );
    ogl->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
}

void fe_glcanvas::draw( ogl_context* ogl )
{
    rect viewport = ogl_bounds();

    if ( texture_width < viewport.width() || texture_height < viewport.height() )
    {
        texture_width = (GLsizei)viewport.width();
        texture_height = (GLsizei)viewport.height();
        ogl->glBindTexture( GL_TEXTURE_2D, texture );
        ogl->glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
        ogl->glBindTexture( GL_TEXTURE_2D, 0 );
    }
    
    ogl->glBindFramebuffer( GL_FRAMEBUFFER, fbo );

    ogl->glViewport( 0.0f, 0.0f, viewport.width(), viewport.height() );
    ogl->glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    ogl->glClear( GL_COLOR_BUFFER_BIT );
    
    // Transform into viewport coordinates (same coordinates as gl_FragCoord,
    // origin lower left, one pixel is one unit).
    float s = ( 12.0f / emsize ) * scale;
    matrix3 view
    (
        s, 0.0f, 0.0f,
        0.0f, s, 0.0f,
        offset.x, offset.y, 1.0f
    );


    
    ogl->glEnable( GL_BLEND );
    ogl->glBlendEquation( GL_FUNC_ADD );
    ogl->glBlendFunc( GL_ONE, GL_ONE );
    
    ogl->glUseProgram( program );
    ogl->glUniform2f( u_viewport, viewport.width(), viewport.height() );
    
    ogl->glBindVertexArray( vao );
    
    float2 p = float2( 0.0f, 0.0f );
    char32_t prev = 0;
    for ( const char* j = jabberwocky; *j; ++j )
    {
        char32_t c = *j;
        
        if ( c == '\n' )
        {
            p.x = 0.0f;
            p.y -= line_height;
            continue;
        }
        
        
        uint64_t key = (uint64_t)prev << 32 | (uint64_t)c;
        auto k = kerning.find( key );
        if ( k != kerning.end() )
        {
            p.x += k->second;
        }
        prev = c;
        
        
        matrix3 model
        (
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            p.x,  p.y,  1.0f
        );
        
        matrix3 tf = model * view;
        ogl->glUniformMatrix3fv( u_transform, 1, GL_FALSE, &tf[ 0 ][ 0 ] );
        
        const glyph& g = glyphs.at( *j );
        ogl->glDrawElements( GL_TRIANGLES, g.count, GL_UNSIGNED_INT, g.indices );

        p.x += g.advance;

    }
    
    
    ogl->glBindVertexArray( 0 );
    ogl->glUseProgram( 0 );
    ogl->glDisable( GL_BLEND );
    
    ogl->glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    ogl->glViewport( 0.0f, 0.0f, viewport.width(), viewport.height() );
    
    float tw = viewport.width() / texture_width;
    float th = viewport.height() / texture_height;
    
    const blit_vertex v[] =
    {
        { float2( -1.0f, -1.0f ), float2( 0.0f, 0.0f ) },
        { float2(  1.0f, -1.0f ), float2( tw, 0.0f ) },
        { float2( -1.0f,  1.0f ), float2( 0.0f, th ) },
        { float2(  1.0f,  1.0f ), float2( tw, th ) }
    };
    
    ogl->glBindBuffer( GL_ARRAY_BUFFER, blit_vbo );
    ogl->glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( v ), v );
    ogl->glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    ogl->glUseProgram( blit );
    
    ogl->glActiveTexture( GL_TEXTURE0 );
    ogl->glBindTexture( GL_TEXTURE_2D, texture );
    
    ogl->glBindVertexArray( blit_vao );
    ogl->glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0 );
    ogl->glBindVertexArray( 0 );
    
    ogl->glBindTexture( GL_TEXTURE_2D, 0 );
    ogl->glUseProgram( 0 );

    
}


void fe_glcanvas::on_mouse_down( float2 p, uic_button button )
{
    drag_active = true;
    drag_start = p;
}

void fe_glcanvas::on_mouse_up( float2 p, uic_button button )
{
    drag_active = false;
}

void fe_glcanvas::on_mouse_move( float2 p )
{
    if ( drag_active )
    {
        offset += ( p - drag_start );
        offset.x = roundf( offset.x );
        offset.y = roundf( offset.y );
        drag_start = p;
        invalidate();
    }
}

void fe_glcanvas::on_scroll( float2 p, float amount )
{
    scale += amount * 0.05f;
    scale = clamp( scale, 0.1f, 20.0f );
    invalidate();
}

void fe_glcanvas::on_zoom( float2 p, float amount )
{
    scale *= amount;
    scale = clamp( scale, 0.1f, 20.0f );
    invalidate();
}




int main( int argc, const char* argv[] )
{
    if ( argc < 2 )
    {
        fprintf( stderr, "usage: %s <font-file>\n",
                    path_filename( argv[ 0 ] ).c_str() );
        return EXIT_FAILURE;
    }

    auto application = std::make_unique< uic_application >();
    auto window = std::make_unique< uic_window >();
    window->set_widget( std::make_shared< fe_glcanvas >( argv[ 1 ] ) );
    window->show();
    application->eventloop();

    return EXIT_SUCCESS;
}




