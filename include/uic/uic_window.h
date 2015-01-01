//
//  uic_window.h
//
//  Created by Edmund Kapusniak on 18/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef UIC_WINDOW_H
#define UIC_WINDOW_H


#include "uic_widget.h"


struct uic_window_impl;


/*
    A top-level application window.
*/

class uic_window
{
public:

    uic_window();
    ~uic_window();
    
    void show();
    void hide();
    
    void set_widget( const uic_widget_p& widget );
    
    
private:

    std::unique_ptr< uic_window_impl > p;

};



#endif
