//
//  ogl_version.h
//
//  Created by Edmund Kapusniak on 01/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef OGL_VERSION_H
#define OGL_VERSION_H


#include "ogl_headers.h"


struct ogl_version
{
    ogl_version();
    ogl_version( GLint major, GLint minor );

    GLint major;
    GLint minor;
};


inline bool operator == ( const ogl_version& a, const ogl_version& b )
{
    return a.major == b.major && a.minor == b.minor;
}

inline bool operator != ( const ogl_version& a, const ogl_version& b )
{
    return !( a == b );
}

inline bool operator < ( const ogl_version& a, const ogl_version& b )
{
    return a.major < b.major || ( a.major == b.major && a.minor < b.minor );
}

inline bool operator <= ( const ogl_version& a, const ogl_version& b )
{
    return a.major <= b.major || ( a.major == b.major && a.minor <= b.minor );
}

inline bool operator > ( const ogl_version& a, const ogl_version& b )
{
    return !( a <= b );
}

inline bool operator >= ( const ogl_version& a, const ogl_version& b )
{
    return !( a < b );
}


inline ogl_version::ogl_version()
    :   major( 0 )
    ,   minor( 0 )
{
}

inline ogl_version::ogl_version( GLint major, GLint minor )
    :   major( major )
    ,   minor( minor )
{
}


#endif
