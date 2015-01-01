//
//  uic_application.mm
//
//  Created by Edmund Kapusniak on 18/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#include "uic_application.h"
#import <AppKit/NSApplication.h>



uic_application::uic_application()
{
    // http://stackoverflow.com/questions/8137538/cocoa-applications-from-the-command-line
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType( &psn, kProcessTransformToForegroundApplication );
    
    // Create NSApplication singleton.
    [NSApplication sharedApplication];
}

uic_application::~uic_application()
{
}


void uic_application::eventloop()
{
    [NSApp run];
}












