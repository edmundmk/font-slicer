//
//  rect.h
//
//  Created by Edmund Kapusniak on 05/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef RECT_H
#define RECT_H


#include <math3.h>


/*
    A rectangle.
*/

struct rect
{
    rect();
    rect( float minx, float miny, float maxx, float maxy );
    
    float   width() const;
    float   height() const;
    float2  centre() const;
    
    float minx;
    float miny;
    float maxx;
    float maxy;
    
};


/*
    A convex four-sided polygon, allowing arbitrarily transformed rectangles
    to be represented.  Point order is anticlockwise.
*/

class quad
{

    quad();
    quad( const rect& r );
    quad( float minx, float miny, float maxx, float maxy );
    quad( float2 q0, float2 q1, float2 q2, float2 q3 );

    float2 q[ 4 ];
    
};



/*

*/


inline rect::rect()
    :   minx( F_NAN )
    ,   miny( F_NAN )
    ,   maxx( F_NAN )
    ,   maxy( F_NAN )
{
}

inline rect::rect( float minx, float miny, float maxx, float maxy )
    :   minx( minx )
    ,   miny( miny )
    ,   maxx( maxx )
    ,   maxy( maxy )
{
}

inline float rect::width() const
{
    return maxx - minx;
}

inline float rect::height() const
{
    return maxy - miny;
}

inline float2 rect::centre() const
{
    return float2( ( maxx - minx ) * 0.5f, ( maxy - miny ) * 0.5f );
}


#endif
