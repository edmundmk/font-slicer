//
//  uic_glcanvas.mm
//
//  Created by Edmund Kapusniak on 29/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#include "uic_glcanvas.h"
#include <make_unique.h>
#include <ogl/ogl_context.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSOpenGLView.h>


@class UICOpenGLView;


struct uic_glcanvas_impl
{
    uic_glcanvas* outer;
    UICOpenGLView*  view;
    std::unique_ptr< ogl_context > ogl;

    void setup_context();
    void draw();

    void on_mouse_down( float2 p, uic_button button );
    void on_mouse_up( float2 p, uic_button button );
    void on_mouse_move( float2 p );
    void on_scroll( float2 p, float amount );
    void on_zoom( float2 p, float amount );

};


@interface UICOpenGLView : NSOpenGLView
{
    uic_glcanvas_impl* control;
}

-(instancetype)initWithControl:(uic_glcanvas_impl*)_control;
-(void)prepareOpenGL;
-(void)drawRect:(NSRect)rect;

-(void)mouseDown:(NSEvent*)event;
-(void)mouseDragged:(NSEvent*)event;
-(void)mouseUp:(NSEvent*)event;
-(void)scrollWheel:(NSEvent*)event;
-(void)magnifyWithEvent:(NSEvent*)event;

@end


uic_glcanvas::uic_glcanvas()
    :   p( new uic_glcanvas_impl() )
{
    p->outer = this;
    p->view = [[UICOpenGLView alloc] initWithControl:p.get()];
}

uic_glcanvas::~uic_glcanvas()
{
}


void uic_glcanvas::reparent( void* parent )
{
    NSView* view = (__bridge NSView*)parent;
    if ( view )
        [view addSubview:p->view];
    else
        [p->view removeFromSuperview];
}

void uic_glcanvas::layout( const rect& rect )
{
    NSRect frame = NSMakeRect
    (
        rect.minx,
        rect.miny,
        rect.maxx - rect.minx,
        rect.maxy - rect.miny
    );
    [p->view setFrame:frame];
}


void uic_glcanvas::invalidate()
{
    [p->view setNeedsDisplay:YES];
}


rect uic_glcanvas::bounds()
{
    NSRect frame = [p->view frame];
    return rect
    (
        NSMinX( frame ),
        NSMinY( frame ),
        NSMaxX( frame ),
        NSMaxY( frame )
    );
}

rect uic_glcanvas::ogl_bounds()
{
    NSRect frame = [p->view convertRectToBacking:[p->view frame]];
    return rect
    (
        NSMinX( frame ),
        NSMinY( frame ),
        NSMaxX( frame ),
        NSMaxY( frame )
    );
}


void uic_glcanvas::setup_context( ogl_context* ogl )
{
}

void uic_glcanvas::draw( ogl_context* ogl )
{
}


void uic_glcanvas_impl::setup_context()
{
    assert( ! ogl );
    ogl = std::make_unique< ogl_context >();
    outer->setup_context( ogl.get() );
}

void uic_glcanvas_impl::draw()
{
    outer->draw( ogl.get() );
}

void uic_glcanvas_impl::on_mouse_down( float2 p, uic_button button )
{
    outer->on_mouse_down( p, button );
}

void uic_glcanvas_impl::on_mouse_up( float2 p, uic_button button )
{
    outer->on_mouse_up( p, button );
}

void uic_glcanvas_impl::on_mouse_move( float2 p )
{
    outer->on_mouse_move( p );
}

void uic_glcanvas_impl::on_scroll( float2 p, float amount )
{
    outer->on_scroll( p, amount );
}

void uic_glcanvas_impl::on_zoom( float2 p, float amount )
{
    outer->on_zoom( p, amount );
}



@implementation UICOpenGLView

-(instancetype)initWithControl:(uic_glcanvas_impl*)_control
{
    control = _control;

    NSOpenGLPixelFormatAttribute attributes[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        0
    };
    NSOpenGLPixelFormat* format =
            [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];

    self = [self initWithFrame:NSMakeRect( 0, 0, 10, 10 ) pixelFormat:format];
    [self setWantsBestResolutionOpenGLSurface:YES];

    return self;
}


-(void)prepareOpenGL
{
    control->setup_context();
}


-(void)drawRect:(NSRect)rect
{
    control->draw();
    [[self openGLContext] flushBuffer];
}


-(void)mouseDown:(NSEvent*)event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    control->on_mouse_down( float2( p.x, p.y ), UIC_BUTTON_LEFT );
    [super mouseDown:event];
}

-(void)mouseDragged:(NSEvent*)event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    control->on_mouse_move( float2( p.x, p.y ) );
    [super mouseDragged:event];
}

-(void)mouseUp:(NSEvent*)event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    control->on_mouse_up( float2( p.x, p.y ), UIC_BUTTON_LEFT );
    [super mouseUp:event];
}

-(void)scrollWheel:(NSEvent*)event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    control->on_scroll( float2( p.x, p.y ), [event deltaY] );
    [super scrollWheel:event];
}

-(void)magnifyWithEvent:(NSEvent*)event
{
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    control->on_zoom( float2( p.x, p.y ), 1.0f + [event magnification] );
    [super magnifyWithEvent:event];
}


@end

