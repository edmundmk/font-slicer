//
//  ogl_context.h
//
//  Created by Edmund Kapusniak on 27/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef OGL_CONTEXT_H
#define OGL_CONTEXT_H


#include <memory>
#include "ogl_headers.h"


struct ogl_context_impl;


// OpenGL 3.2 Core
#define GL_RED                          0x1903
#define GL_RG                           0x8227

// OES_vertex_array_object
#define GL_VERTEX_ARRAY_BINDING         0x85B5

// OES_mapbuffer
#define GL_WRITE_ONLY                   0x88B9
#define GL_BUFFER_ACCESS                0x88BB
#define GL_BUFFER_MAPPED                0x88BC
#define GL_BUFFER_MAP_POINTER           0x88BD

// EXT_map_buffer_range
#define GL_MAP_READ_BIT                 0x0001
#define GL_MAP_WRITE_BIT                0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT     0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT    0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT       0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT       0x0020

// EXT_sRGB
#define GL_SRGB                         0x8C40
#define GL_SRGB_ALPHA                   0x8C42
#define GL_SRGB8_ALPHA8                 0x8C43
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210

// EXT_debug_label
#define GL_BUFFER_OBJECT                0x9151
#define GL_SHADER_OBJECT                0x8B48
#define GL_PROGRAM_OBJECT               0x8B40
#define GL_VERTEX_ARRAY_OBJECT          0x9154
#define GL_QUERY_OBJECT                 0x9153
#define GL_PROGRAM_PIPELINE_OBJECT      0x8A4F



/*
    Not all differences between desktop GL core and ES2 can be easily emulated.
    For example, one-channel textures are GL_ALPHA or GL_LUMINANCE in ES2 but
    must be GL_RED on desktop, and two-channel textures are GL_LUMINANCE_ALPHA
    in ES2 but must be GL_RG on desktop.
*/


enum ogl_kind
{
    OGL_CORE,   // Desktop OpenGL 3.2 Core.
    OGL_ES2,    // OpenGL ES 2.0.
};



class ogl_context
{
public:

    ogl_context();


    // Useful helper functions.
    GLuint compile_shader( GLenum type, const char* source, size_t length = 0 );
    void link_program( GLuint program );


    // Which kind of context do we have?
    ogl_kind kind;


    // Optional OpenGL ES 2.0 extensions.
    bool OES_element_index_uint;
    bool OES_mapbuffer;
    bool EXT_map_buffer_range;
    bool EXT_sRGB;


    // Common subset.
    void (*glActiveTexture)( GLenum texture );
    void (*glAttachShader)( GLuint program, GLuint shader );
    void (*glBindAttribLocation)( GLuint program, GLuint index, const GLchar *name );
    void (*glBindBuffer)( GLenum target, GLuint buffer );
    void (*glBindFramebuffer)( GLenum target, GLuint framebuffer );
    void (*glBindRenderbuffer)( GLenum target, GLuint renderbuffer );
    void (*glBindTexture)( GLenum target, GLuint texture );
    void (*glBlendColor)( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
    void (*glBlendEquation)( GLenum mode );
    void (*glBlendEquationSeparate)( GLenum modeRGB, GLenum modeAlpha );
    void (*glBlendFunc)( GLenum sfactor, GLenum dfactor );
    void (*glBlendFuncSeparate)( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha );
    void (*glBufferData)( GLenum target, GLsizeiptr size, const void *data, GLenum usage );
    void (*glBufferSubData)( GLenum target, GLintptr offset, GLsizeiptr size, const void *data );
    GLenum (*glCheckFramebufferStatus)( GLenum target );
    void (*glClear)( GLbitfield mask );
    void (*glClearColor)( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
    void (*glClearStencil)( GLint s );
    void (*glColorMask)( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );
    void (*glCompileShader)( GLuint shader );
    void (*glCompressedTexImage2D)( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data );
    void (*glCompressedTexSubImage2D)( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data );
    void (*glCopyTexImage2D)( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border );
    void (*glCopyTexSubImage2D)( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height );
    GLuint (*glCreateProgram)();
    GLuint (*glCreateShader)( GLenum type );
    void (*glCullFace)( GLenum mode );
    void (*glDeleteBuffers)( GLsizei n, const GLuint *buffers );
    void (*glDeleteFramebuffers)( GLsizei n, const GLuint *framebuffers );
    void (*glDeleteProgram)( GLuint program );
    void (*glDeleteRenderbuffers)( GLsizei n, const GLuint *renderbuffers );
    void (*glDeleteShader)( GLuint shader );
    void (*glDeleteTextures)( GLsizei n, const GLuint *textures );
    void (*glDepthFunc)( GLenum func );
    void (*glDepthMask)( GLboolean flag );
    void (*glDetachShader)( GLuint program, GLuint shader );
    void (*glDisable)( GLenum cap );
    void (*glDisableVertexAttribArray)( GLuint index );
    void (*glDrawArrays)( GLenum mode, GLint first, GLsizei count );
    void (*glDrawElements)( GLenum mode, GLsizei count, GLenum type, const void *indices );
    void (*glEnable)( GLenum cap );
    void (*glEnableVertexAttribArray)( GLuint index );
    void (*glFinish)();
    void (*glFlush)();
    void (*glFramebufferRenderbuffer)( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer );
    void (*glFramebufferTexture2D)( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level );
    void (*glFrontFace)( GLenum mode );
    void (*glGenBuffers)( GLsizei n, GLuint *buffers );
    void (*glGenerateMipmap)( GLenum target );
    void (*glGenFramebuffers)( GLsizei n, GLuint *framebuffers );
    void (*glGenRenderbuffers)( GLsizei n, GLuint *renderbuffers );
    void (*glGenTextures)( GLsizei n, GLuint *textures );
    void (*glGetActiveAttrib)( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name );
    void (*glGetActiveUniform)( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name );
    void (*glGetAttachedShaders)( GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders );
    GLint (*glGetAttribLocation)( GLuint program, const GLchar *name );
    void (*glGetBooleanv)( GLenum pname, GLboolean *data );
    void (*glGetBufferParameteriv)( GLenum target, GLenum pname, GLint *params );
    GLenum (*glGetError)();
    void (*glGetFloatv)( GLenum pname, GLfloat *data );
    void (*glGetFramebufferAttachmentParameteriv)( GLenum target, GLenum attachment, GLenum pname, GLint *params );
    void (*glGetIntegerv)( GLenum pname, GLint *data );
    void (*glGetProgramiv)( GLuint program, GLenum pname, GLint *params );
    void (*glGetProgramInfoLog)( GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog );
    void (*glGetRenderbufferParameteriv)( GLenum target, GLenum pname, GLint *params );
    void (*glGetShaderiv)( GLuint shader, GLenum pname, GLint *params );
    void (*glGetShaderInfoLog)( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog );
    void (*glGetShaderSource)( GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source );
    const GLubyte *(*glGetString)( GLenum name );
    void (*glGetTexParameterfv)( GLenum target, GLenum pname, GLfloat *params );
    void (*glGetTexParameteriv)( GLenum target, GLenum pname, GLint *params );
    void (*glGetUniformfv)( GLuint program, GLint location, GLfloat *params );
    void (*glGetUniformiv)( GLuint program, GLint location, GLint *params );
    GLint (*glGetUniformLocation)( GLuint program, const GLchar *name );
    void (*glGetVertexAttribfv)( GLuint index, GLenum pname, GLfloat *params );
    void (*glGetVertexAttribiv)( GLuint index, GLenum pname, GLint *params );
    void (*glGetVertexAttribPointerv)( GLuint index, GLenum pname, void **pointer );
    void (*glHint)( GLenum target, GLenum mode );
    GLboolean (*glIsBuffer)( GLuint buffer );
    GLboolean (*glIsEnabled)( GLenum cap );
    GLboolean (*glIsFramebuffer)( GLuint framebuffer );
    GLboolean (*glIsProgram)( GLuint program );
    GLboolean (*glIsRenderbuffer)( GLuint renderbuffer );
    GLboolean (*glIsShader)( GLuint shader );
    GLboolean (*glIsTexture)( GLuint texture );
    void (*glLineWidth)( GLfloat width );
    void (*glLinkProgram)( GLuint program );
    void (*glPixelStorei)( GLenum pname, GLint param );
    void (*glPolygonOffset)( GLfloat factor, GLfloat units );
    void (*glReadPixels)( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels );
    void (*glRenderbufferStorage)( GLenum target, GLenum internalformat, GLsizei width, GLsizei height );
    void (*glSampleCoverage)( GLfloat value, GLboolean invert );
    void (*glScissor)( GLint x, GLint y, GLsizei width, GLsizei height );
    void (*glShaderSource)( GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length );
    void (*glStencilFunc)( GLenum func, GLint ref, GLuint mask );
    void (*glStencilFuncSeparate)( GLenum face, GLenum func, GLint ref, GLuint mask );
    void (*glStencilMask)( GLuint mask );
    void (*glStencilMaskSeparate)( GLenum face, GLuint mask );
    void (*glStencilOp)( GLenum fail, GLenum zfail, GLenum zpass );
    void (*glStencilOpSeparate)( GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass );
    void (*glTexImage2D)( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels );
    void (*glTexParameterf)( GLenum target, GLenum pname, GLfloat param );
    void (*glTexParameterfv)( GLenum target, GLenum pname, const GLfloat *params );
    void (*glTexParameteri)( GLenum target, GLenum pname, GLint param );
    void (*glTexParameteriv)( GLenum target, GLenum pname, const GLint *params );
    void (*glTexSubImage2D)( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels );
    void (*glUniform1f)( GLint location, GLfloat v0 );
    void (*glUniform1fv)( GLint location, GLsizei count, const GLfloat *value );
    void (*glUniform1i)( GLint location, GLint v0 );
    void (*glUniform1iv)( GLint location, GLsizei count, const GLint *value );
    void (*glUniform2f)( GLint location, GLfloat v0, GLfloat v1 );
    void (*glUniform2fv)( GLint location, GLsizei count, const GLfloat *value );
    void (*glUniform2i)( GLint location, GLint v0, GLint v1 );
    void (*glUniform2iv)( GLint location, GLsizei count, const GLint *value );
    void (*glUniform3f)( GLint location, GLfloat v0, GLfloat v1, GLfloat v2 );
    void (*glUniform3fv)( GLint location, GLsizei count, const GLfloat *value );
    void (*glUniform3i)( GLint location, GLint v0, GLint v1, GLint v2 );
    void (*glUniform3iv)( GLint location, GLsizei count, const GLint *value );
    void (*glUniform4f)( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 );
    void (*glUniform4fv)( GLint location, GLsizei count, const GLfloat *value );
    void (*glUniform4i)( GLint location, GLint v0, GLint v1, GLint v2, GLint v3 );
    void (*glUniform4iv)( GLint location, GLsizei count, const GLint *value );
    void (*glUniformMatrix2fv)( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value );
    void (*glUniformMatrix3fv)( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value );
    void (*glUniformMatrix4fv)( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value );
    void (*glUseProgram)( GLuint program );
    void (*glValidateProgram)( GLuint program );
    void (*glVertexAttrib1f)( GLuint index, GLfloat x );
    void (*glVertexAttrib1fv)( GLuint index, const GLfloat *v );
    void (*glVertexAttrib2f)( GLuint index, GLfloat x, GLfloat y );
    void (*glVertexAttrib2fv)( GLuint index, const GLfloat *v );
    void (*glVertexAttrib3f)( GLuint index, GLfloat x, GLfloat y, GLfloat z );
    void (*glVertexAttrib3fv)( GLuint index, const GLfloat *v );
    void (*glVertexAttrib4f)( GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w );
    void (*glVertexAttrib4fv)( GLuint index, const GLfloat *v );
    void (*glVertexAttribPointer)( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer );
    void (*glViewport)( GLint x, GLint y, GLsizei width, GLsizei height );

    // ARB_es2_compatibility
    void (*glClearDepthf)( GLfloat d );
    void (*glDepthRangef)( GLfloat n, GLfloat f );
    void (*glGetShaderPrecisionFormat)( GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision );
    void (*glReleaseShaderCompiler)();
    void (*glShaderBinary)( GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length );

    // OES_vertex_array_object
    void (*glBindVertexArray)( GLuint array );
    void (*glDeleteVertexArrays)( GLsizei n, const GLuint *arrays );
    void (*glGenVertexArrays)( GLsizei n, GLuint *arrays );
    GLboolean (*glIsVertexArray)( GLuint array );
    
    // OES_mapbuffer
    void (*glGetBufferPointerv)( GLenum target, GLenum pname, GLvoid** params );
    GLvoid* (*glMapBuffer)( GLenum target, GLenum access );
    GLboolean (*glUnmapBuffer)( GLenum target );
    
    // EXT_map_buffer_range
    GLvoid* (*glMapBufferRange)( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access );
    void (*glFlushMappedBufferRange)( GLenum target, GLintptr offset, GLsizeiptr length );
    
    // EXT_debug_label
    void (*glLabelObject)( GLenum type, GLuint object, GLsizei length, const GLchar *label );
    void (*glGetObjectLabel)( GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label );
    
    // EXT_debug_marker
    void (*glInsertEventMarker)( GLsizei length, const GLchar *marker );
    void (*glPushGroupMarker)( GLsizei length, const GLchar *marker );
    void (*glPopGroupMarker)();


private:

    void (*glBindFragDataLocation)(	GLuint program, GLuint colorNumber, const GLchar *name );
     
};



#endif
