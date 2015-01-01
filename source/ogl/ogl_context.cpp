//
//  ogl_context.cpp
//
//  Created by Edmund Kapusniak on 26/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#include "ogl_context.h"
#include <vector>



GLuint ogl_context::compile_shader(
            GLenum type, const char *source, size_t length )
{
    GLuint shader = glCreateShader( type );

    const GLchar* preamble = "";
    if ( type == GL_VERTEX_SHADER )
    {
        preamble =
            "#version 150\n"
            "#define attribute in\n"
            "#define varying out\n"
            "#define texture2D texture\n"
            ;
    }
    else if ( type == GL_FRAGMENT_SHADER )
    {
        preamble =
            "#version 150\n"
            "#define varying in\n"
            "#define texture2D texture\n"
            "#define gl_FragColor _out_color\n"
            "out vec4 _out_color;\n"
            ;
    }

    const GLchar* sources[] =
    {
        preamble,
        source
    };
    GLint lengths[] =
    {
        -1,
        length ? (GLint)length : -1
    };
    glShaderSource( shader, 2, sources, lengths );
    glCompileShader( shader );
    
    GLint status = GL_FALSE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if ( ! status )
    {
        GLint length = 0;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &length );
        std::vector< GLchar > log;
        log.resize( length );
        glGetShaderInfoLog( shader, (GLsizei)log.size(), NULL, log.data() );
        fprintf( stderr, "%s\n", log.data() );
    }
    
    return shader;
}



void ogl_context::link_program( GLuint program )
{
    glBindFragDataLocation( program, 0, "_out_color" );

    glLinkProgram( program );
    
    GLint status = GL_FALSE;
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if ( ! status )
    {
        GLint length = 0;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &length );
        std::vector< GLchar > log;
        log.resize( length );
        glGetProgramInfoLog( program, (GLsizei)log.size(), NULL, log.data() );
        fprintf( stderr, "%s\n", log.data() );
    }
}


