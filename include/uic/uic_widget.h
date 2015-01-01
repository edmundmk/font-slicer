//
//  uic_widget.h
//
//  Created by Edmund Kapusniak on 27/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef UIC_WIDGET_H
#define UIC_WIDGET_H


#include <memory>
#include <rect.h>


class uic_window;


/*
    A widget is an element of the user interface that participates in layout.
    It has two subclasses - controls display information and react to user
    interaction, while layouts manage the size and position of several sub-
    widgets.
*/

class uic_widget
{
public:

    uic_widget();
    virtual ~uic_widget();


protected:

    friend struct uic_window_impl;

    virtual void reparent( void* parent );
    virtual void layout( const rect& rect );
    

};


typedef std::shared_ptr< uic_widget > uic_widget_p;



#endif
