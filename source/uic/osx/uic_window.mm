//
//  uic_window.mm
//
//  Created by Edmund Kapusniak on 18/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#include "uic_window.h"
#import <AppKit/NSWindow.h>
#import <AppKit/NSView.h>



@class uic_window_delegate;



struct uic_window_impl
{
    NSWindow*               window;
    uic_window_delegate*    delegate;
    uic_widget_p            widget;

    void set_widget( const uic_widget_p& _widget );
    void layout();

};




@interface uic_window_delegate : NSObject< NSWindowDelegate >
{
    uic_window_impl* window;
}

-(instancetype)initWithWindow:(uic_window_impl*)_window;
-(void)windowDidResize:(NSNotification*)notification;

@end



uic_window::uic_window()
    :   p( new uic_window_impl() )
{
    p->window = [[NSWindow alloc]
            initWithContentRect: NSMakeRect( 0, 0, 1280, 720 )
            styleMask: NSTitledWindowMask | NSClosableWindowMask
                        | NSMiniaturizableWindowMask | NSResizableWindowMask
//                        | NSTexturedBackgroundWindowMask
            backing: NSBackingStoreBuffered
            defer: NO
        ];
    [p->window setCollectionBehavior: [p->window collectionBehavior]
            | NSWindowCollectionBehaviorFullScreenPrimary ];
    [[p->window contentView] setAutoresizesSubviews:NO];

    p->delegate = [[uic_window_delegate alloc] initWithWindow:p.get()];
    [p->window setDelegate: p->delegate];
}

uic_window::~uic_window()
{
}


void uic_window::show()
{
    [p->window makeKeyAndOrderFront:nil];
}

void uic_window::hide()
{
    [p->window orderOut:nil];
}


void uic_window::set_widget( const uic_widget_p& widget )
{
    p->set_widget( widget );
}



void uic_window_impl::set_widget( const uic_widget_p& _widget )
{
    if ( widget )
    {
        widget->reparent( nullptr );
    }
    widget = _widget;
    if ( widget )
    {
        widget->reparent( (__bridge void*)[window contentView] );
        layout();
    }
}


void uic_window_impl::layout()
{
    if ( widget )
    {
        NSView* view = [window contentView];
        rect bounds
        (
            view.bounds.origin.x,
            view.bounds.origin.y,
            view.bounds.origin.x + view.bounds.size.width,
            view.bounds.origin.y + view.bounds.size.height
        );
        widget->layout( bounds );
        [view setNeedsDisplay:YES];
    }
}



@implementation uic_window_delegate

-(instancetype)initWithWindow:(uic_window_impl*)_window
{
    window = _window;
    return self;
}

-(void)windowDidResize:(NSNotification*)notification
{
    window->layout();
}

@end








