//
//  uic_glcanvas.h
//
//  Created by Edmund Kapusniak on 28/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef UIC_GLCANVAS_H
#define UIC_GLCANVAS_H


#include <memory>
#include "uic_control.h"


class ogl_context;
struct uic_glcanvas_impl;



/*
    A control which is rendered with OpenGL.
*/

class uic_glcanvas : public uic_control
{
public:

    uic_glcanvas();
    virtual ~uic_glcanvas();
    
    
protected:

    virtual void reparent( void* parent );
    virtual void layout( const rect& rect );

    virtual void invalidate();

    virtual rect bounds();

    virtual void setup_context( ogl_context* ogl );
    virtual void draw( ogl_context* ogl );


private:

    friend struct uic_glcanvas_impl;

    std::unique_ptr< uic_glcanvas_impl > p;

};


#endif