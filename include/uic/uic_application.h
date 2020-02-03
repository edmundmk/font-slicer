//
//  uic_application.h
//
//  Created by Edmund Kapusniak on 18/11/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef UIC_APPLICATION_H
#define UIC_APPLICATION_H



/*
    Main application object and application event loop.
*/

class uic_application
{
public:

    uic_application();
    ~uic_application();

    void eventloop();


};



#endif
