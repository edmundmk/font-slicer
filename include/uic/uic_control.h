//
//  uic_control.h
//
//  Created by Edmund Kapusniak on 28/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef UIC_CONTROL_H
#define UIC_CONTROL_H


#include "uic_widget.h"


enum uic_button
{
    UIC_BUTTON_LEFT,
    UIC_BUTTON_RIGHT,
};



/*
    A control is a widget that is visible and can respond to user interaction.
    Most controls are wrappers around platform windowing system controls.
*/

class uic_control : public uic_widget
{
protected:

    virtual void invalidate();

    virtual rect bounds();

    virtual void on_mouse_down( float2 p, uic_button button );
    virtual void on_mouse_up( float2 p, uic_button button );
    virtual void on_mouse_move( float2 p );
    virtual void on_scroll( float2 p, float amount );
    virtual void on_zoom( float2 p, float amount );

};


#endif
