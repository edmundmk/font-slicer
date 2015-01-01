//
//  ogl_context_osx.cpp
//
//  Created by Edmund Kapusniak on 01/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#include "ogl_context.h"
#include <unordered_set>
#include <symkey.h>
#include "ogl_exception.h"
#include "ogl_version.h"


#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>




// OpenGL ES 2.0 emulation.

static void oglClearDepthf( GLfloat d )
{
    ::glClearDepth( d );
}

static void oglDepthRangef( GLfloat n, GLfloat f )
{
    ::glDepthRange( n, f );
}

static void oglGetShaderPrecisionFormat( GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision )
{
    switch ( precisiontype )
    {
    case GL_HIGH_FLOAT:
    case GL_MEDIUM_FLOAT:
    case GL_LOW_FLOAT:
        if ( range )
            range[ 0 ] = 127, range[ 1 ] = 127;
        if ( precision )
            *precision = 23;
        break;
    case GL_HIGH_INT:
    case GL_MEDIUM_INT:
    case GL_LOW_INT:
        if ( range )
            range[ 0 ] = 31, range[ 1 ] = 30;
        if ( precision )
            *precision = 0;
        break;
    }
}

static void oglReleaseShaderCompiler()
{
}

static void oglShaderBinary( GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length )
{
    throw ogl_exception( "glShaderBinary unsupported" );
}




// EXT_debug_label emulation.

static void oglLabelObject( GLenum type, GLuint object, GLsizei length, const GLchar *label )
{
}

static void oglGetObjectLabel( GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label )
{
    if ( bufSize )
        label[ 0 ] = '\0';
    if ( length )
        *length = 0;
}


// EXT_debug_marker emulation.

static void oglInsertEventMarker( GLsizei length, const GLchar *marker )
{
}

static void oglPushGroupMarker( GLsizei length, const GLchar *marker )
{
}

static void oglPopGroupMarker()
{
}




ogl_context::ogl_context()
{
    memset( this, 0, sizeof( ogl_context ) );


    kind = OGL_CORE;
    

    ogl_version version;
    ::glGetIntegerv( GL_MAJOR_VERSION, &version.major );
    ::glGetIntegerv( GL_MINOR_VERSION, &version.minor );
    
    std::unordered_set< symkey > extensions;
    GLint num_extensions = 0;
    ::glGetIntegerv( GL_NUM_EXTENSIONS, &num_extensions );
    
    for ( GLint i = 0; i < num_extensions; ++i )
    {
        const GLubyte* extension = ::glGetStringi( GL_EXTENSIONS, i );
        extensions.emplace( (const char*)extension );
    }
    

    if ( version >= ogl_version( 3, 2 ) )
    {
    
        glActiveTexture = ::glActiveTexture;
        glAttachShader = ::glAttachShader;
        glBindAttribLocation = ::glBindAttribLocation;
        glBindBuffer = ::glBindBuffer;
        glBindFramebuffer = ::glBindFramebuffer;
        glBindRenderbuffer = ::glBindRenderbuffer;
        glBindTexture = ::glBindTexture;
        glBlendColor = ::glBlendColor;
        glBlendEquation = ::glBlendEquation;
        glBlendEquationSeparate = ::glBlendEquationSeparate;
        glBlendFunc = ::glBlendFunc;
        glBlendFuncSeparate = ::glBlendFuncSeparate;
        glBufferData = ::glBufferData;
        glBufferSubData = ::glBufferSubData;
        glCheckFramebufferStatus = ::glCheckFramebufferStatus;
        glClear = ::glClear;
        glClearColor = ::glClearColor;
        glClearStencil = ::glClearStencil;
        glColorMask = ::glColorMask;
        glCompileShader = ::glCompileShader;
        glCompressedTexImage2D = ::glCompressedTexImage2D;
        glCompressedTexSubImage2D = ::glCompressedTexSubImage2D;
        glCopyTexImage2D = ::glCopyTexImage2D;
        glCopyTexSubImage2D = ::glCopyTexSubImage2D;
        glCreateProgram = ::glCreateProgram;
        glCreateShader = ::glCreateShader;
        glCullFace = ::glCullFace;
        glDeleteBuffers = ::glDeleteBuffers;
        glDeleteFramebuffers = ::glDeleteFramebuffers;
        glDeleteProgram = ::glDeleteProgram;
        glDeleteRenderbuffers = ::glDeleteRenderbuffers;
        glDeleteShader = ::glDeleteShader;
        glDeleteTextures = ::glDeleteTextures;
        glDepthFunc = ::glDepthFunc;
        glDepthMask = ::glDepthMask;
        glDetachShader = ::glDetachShader;
        glDisable = ::glDisable;
        glDisableVertexAttribArray = ::glDisableVertexAttribArray;
        glDrawArrays = ::glDrawArrays;
        glDrawElements = ::glDrawElements;
        glEnable = ::glEnable;
        glEnableVertexAttribArray = ::glEnableVertexAttribArray;
        glFinish = ::glFinish;
        glFlush = ::glFlush;
        glFramebufferRenderbuffer = ::glFramebufferRenderbuffer;
        glFramebufferTexture2D = ::glFramebufferTexture2D;
        glFrontFace = ::glFrontFace;
        glGenBuffers = ::glGenBuffers;
        glGenerateMipmap = ::glGenerateMipmap;
        glGenFramebuffers = ::glGenFramebuffers;
        glGenRenderbuffers = ::glGenRenderbuffers;
        glGenTextures = ::glGenTextures;
        glGetActiveAttrib = ::glGetActiveAttrib;
        glGetActiveUniform = ::glGetActiveUniform;
        glGetAttachedShaders = ::glGetAttachedShaders;
        glGetAttribLocation = ::glGetAttribLocation;
        glGetBooleanv = ::glGetBooleanv;
        glGetBufferParameteriv = ::glGetBufferParameteriv;
        glGetError = ::glGetError;
        glGetFloatv = ::glGetFloatv;
        glGetFramebufferAttachmentParameteriv = ::glGetFramebufferAttachmentParameteriv;
        glGetIntegerv = ::glGetIntegerv;
        glGetProgramiv = ::glGetProgramiv;
        glGetProgramInfoLog = ::glGetProgramInfoLog;
        glGetRenderbufferParameteriv = ::glGetRenderbufferParameteriv;
        glGetShaderiv = ::glGetShaderiv;
        glGetShaderInfoLog = ::glGetShaderInfoLog;
        glGetShaderSource = ::glGetShaderSource;
        glGetString = ::glGetString;
        glGetTexParameterfv = ::glGetTexParameterfv;
        glGetTexParameteriv = ::glGetTexParameteriv;
        glGetUniformfv = ::glGetUniformfv;
        glGetUniformiv = ::glGetUniformiv;
        glGetUniformLocation = ::glGetUniformLocation;
        glGetVertexAttribfv = ::glGetVertexAttribfv;
        glGetVertexAttribiv = ::glGetVertexAttribiv;
        glGetVertexAttribPointerv = ::glGetVertexAttribPointerv;
        glHint = ::glHint;
        glIsBuffer = ::glIsBuffer;
        glIsEnabled = ::glIsEnabled;
        glIsFramebuffer = ::glIsFramebuffer;
        glIsProgram = ::glIsProgram;
        glIsRenderbuffer = ::glIsRenderbuffer;
        glIsShader = ::glIsShader;
        glIsTexture = ::glIsTexture;
        glLineWidth = ::glLineWidth;
        glLinkProgram = ::glLinkProgram;
        glPixelStorei = ::glPixelStorei;
        glPolygonOffset = ::glPolygonOffset;
        glReadPixels = ::glReadPixels;
        glRenderbufferStorage = ::glRenderbufferStorage;
        glSampleCoverage = ::glSampleCoverage;
        glScissor = ::glScissor;
        glShaderSource = ::glShaderSource;
        glStencilFunc = ::glStencilFunc;
        glStencilFuncSeparate = ::glStencilFuncSeparate;
        glStencilMask = ::glStencilMask;
        glStencilMaskSeparate = ::glStencilMaskSeparate;
        glStencilOp = ::glStencilOp;
        glStencilOpSeparate = ::glStencilOpSeparate;
        glTexImage2D = ::glTexImage2D;
        glTexParameterf = ::glTexParameterf;
        glTexParameterfv = ::glTexParameterfv;
        glTexParameteri = ::glTexParameteri;
        glTexParameteriv = ::glTexParameteriv;
        glTexSubImage2D = ::glTexSubImage2D;
        glUniform1f = ::glUniform1f;
        glUniform1fv = ::glUniform1fv;
        glUniform1i = ::glUniform1i;
        glUniform1iv = ::glUniform1iv;
        glUniform2f = ::glUniform2f;
        glUniform2fv = ::glUniform2fv;
        glUniform2i = ::glUniform2i;
        glUniform2iv = ::glUniform2iv;
        glUniform3f = ::glUniform3f;
        glUniform3fv = ::glUniform3fv;
        glUniform3i = ::glUniform3i;
        glUniform3iv = ::glUniform3iv;
        glUniform4f = ::glUniform4f;
        glUniform4fv = ::glUniform4fv;
        glUniform4i = ::glUniform4i;
        glUniform4iv = ::glUniform4iv;
        glUniformMatrix2fv = ::glUniformMatrix2fv;
        glUniformMatrix3fv = ::glUniformMatrix3fv;
        glUniformMatrix4fv = ::glUniformMatrix4fv;
        glUseProgram = ::glUseProgram;
        glValidateProgram = ::glValidateProgram;
        glVertexAttrib1f = ::glVertexAttrib1f;
        glVertexAttrib1fv = ::glVertexAttrib1fv;
        glVertexAttrib2f = ::glVertexAttrib2f;
        glVertexAttrib2fv = ::glVertexAttrib2fv;
        glVertexAttrib3f = ::glVertexAttrib3f;
        glVertexAttrib3fv = ::glVertexAttrib3fv;
        glVertexAttrib4f = ::glVertexAttrib4f;
        glVertexAttrib4fv = ::glVertexAttrib4fv;
        glVertexAttribPointer = ::glVertexAttribPointer;
        glViewport = ::glViewport;
    
        glBindVertexArray = ::glBindVertexArray;
        glDeleteVertexArrays = ::glDeleteVertexArrays;
        glGenVertexArrays = ::glGenVertexArrays;
        glIsVertexArray = ::glIsVertexArray;

        OES_element_index_uint = true;

        OES_mapbuffer = true;
        glGetBufferPointerv = ::glGetBufferPointerv;
        glMapBuffer = ::glMapBuffer;
        glUnmapBuffer = ::glUnmapBuffer;
    
        EXT_map_buffer_range = true;
        glMapBufferRange = ::glMapBufferRange;
        glFlushMappedBufferRange = ::glFlushMappedBufferRange;
        
        EXT_sRGB = true;
        ::glEnable( GL_FRAMEBUFFER_SRGB );
        
        glBindFragDataLocation = ::glBindFragDataLocation;

    }
    else
    {
        throw ogl_exception( "OpenGL 3.2 is required (current version %d.%d)",
                    version.major, version.minor );
    }


    if ( version >= ogl_version( 4, 1 )
            || extensions.count( "GL_ARB_es2_compatibility" ) )
    {
        glClearDepthf = ::glClearDepthf;
        glDepthRangef = ::glDepthRangef;
        glGetShaderPrecisionFormat = ::glGetShaderPrecisionFormat;
        glReleaseShaderCompiler = ::glReleaseShaderCompiler;
        glShaderBinary = ::glShaderBinary;
    }
    else
    {
        glClearDepthf = ::oglClearDepthf;
        glDepthRangef = ::oglDepthRangef;
        glGetShaderPrecisionFormat = ::oglGetShaderPrecisionFormat;
        glReleaseShaderCompiler = ::oglReleaseShaderCompiler;
        glShaderBinary = ::oglShaderBinary;
    }
    

    if ( extensions.count( "GL_EXT_debug_label" ) )
    {
        glLabelObject = ::glLabelObjectEXT;
        glGetObjectLabel = ::glGetObjectLabelEXT;
    }
    else
    {
        glLabelObject = ::oglLabelObject;
        glGetObjectLabel = ::oglGetObjectLabel;
    }
    
    if ( extensions.count( "GL_EXT_debug_marker" ) )
    {
        glInsertEventMarker = ::glInsertEventMarkerEXT;
        glPushGroupMarker = ::glPushGroupMarkerEXT;
        glPopGroupMarker = ::glPopGroupMarkerEXT;
    }
    else
    {
        glInsertEventMarker = ::oglInsertEventMarker;
        glPushGroupMarker = ::oglPushGroupMarker;
        glPopGroupMarker = ::oglPopGroupMarker;
    }


}


