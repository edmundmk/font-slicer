//
//  rect.h
//
//  Created by Edmund Kapusniak on 05/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef RECT_H
#define RECT_H


#include <algorithm>
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
    
    bool    empty() const;
    bool    contains( float2 p ) const;

    rect    offset( float x, float y ) const;
    rect    inset( float x, float y ) const;

    rect    expand( float2 p ) const;
    rect    expand( const rect& r ) const;
    
    
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
    :   minx( +F_INFINITY )
    ,   miny( +F_INFINITY )
    ,   maxx( -F_INFINITY )
    ,   maxy( -F_INFINITY )
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
    return float2( ( minx + maxx ) * 0.5f, ( miny + maxy ) * 0.5f );
}


inline bool rect::empty() const
{
    return width() <= 0.0f || height() <= 0.0f;
}

inline bool rect::contains( float2 p ) const
{
    return p.x >= minx && p.x < maxx
        && p.y >= miny && p.y < maxy;
}


inline rect rect::offset( float x, float y ) const
{
    return rect( minx + x, miny + y, maxx + x, maxy + y );
}

inline rect rect::inset( float x, float y ) const
{
    return rect( minx + x, miny + y, maxx - x, maxy - y );
}

inline rect rect::expand( float2 p ) const
{
    return rect
    (
        std::min( minx, p.x ),
        std::min( miny, p.y ),
        std::max( maxx, p.x ),
        std::max( maxy, p.y )
    );
}

inline rect rect::expand( const rect& r ) const
{
    return rect
    (
        std::min( minx, r.minx ),
        std::min( miny, r.miny ),
        std::max( maxx, r.maxx ),
        std::max( maxy, r.maxy )
    );
}


#endif
