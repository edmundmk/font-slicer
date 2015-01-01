//
//  bezier.h
//
//  Created by Edmund Kapusniak on 19/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef BEZIER_H
#define BEZIER_H


#include <math3.h>



/*
    A parametric line.
*/

struct lbezier
{
    lbezier();
    lbezier( float2 p0, float2 p1 );
    
    
    float2  evaluate( float t );
    void    split( float t, lbezier out_c[ 2 ] );
    float2  derivative();
    bool    is_monotonic_x();
    bool    is_monotonic_y();
    size_t  solve_x( float x, float* out_t );
    size_t  solve_y( float y, float* out_t );
    
    
    float2 p[ 2 ];
    
};


/*
    A quadratic bezier.
*/

struct qbezier
{
    qbezier();
    qbezier( float2 p0, float2 p1, float2 p2 );


    float2  evaluate( float t );
    void    split( float t, qbezier out_c[ 2 ] );
    lbezier derivative();
    bool    is_monotonic_x();
    bool    is_monotonic_y();
    size_t  solve_x( float x, float out_t[ 2 ] );
    size_t  solve_y( float y, float out_t[ 2 ] );
    

    float2 p[ 3 ];

};


/*
    A cubic bezier.
*/

struct cbezier
{
    cbezier();
    cbezier( float2 p0, float2 p1, float2 p2, float2 p3 );


    float2  evaluate( float t );
    void    split( float t, cbezier out_c[ 2 ] );
    qbezier derivative();
    bool    is_monotonic_x();
    bool    is_monotonic_y();
    size_t  solve_x( float x, float out_t[ 3 ] );
    size_t  solve_y( float y, float out_t[ 3 ] );
    bool    solve_self_intersection( float out_t[ 2 ] );
    

    float2 p[ 4 ];

};





#endif
