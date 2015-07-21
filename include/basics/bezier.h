//
//  bezier.h
//
//  Created by Edmund Kapusniak on 19/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef BEZIER_H
#define BEZIER_H


#include <utility>
#include <math3.h>



/*
    A parametric line.
*/

struct lbezier
{
    lbezier();
    lbezier( float2 p0, float2 p1 );
    
    
    float2  evaluate( float t ) const;
    void    split( float t, lbezier out_c[ 2 ] ) const;
    float2  derivative() const;
    bool    is_monotonic_x() const;
    bool    is_monotonic_y() const;
    size_t  solve_x( float x, float* out_t ) const;
    size_t  solve_y( float y, float* out_t ) const;
    
    
    float2 p[ 2 ];
    
};


/*
    A quadratic bezier.
*/

struct qbezier
{
    qbezier();
    qbezier( const lbezier& l );
    qbezier( float2 p0, float2 p1, float2 p2 );


    float2  evaluate( float t ) const;
    void    split( float t, qbezier out_c[ 2 ] ) const;
    lbezier derivative() const;
    bool    is_monotonic_x() const;
    bool    is_monotonic_y() const;
    size_t  solve_x( float x, float out_t[ 2 ] ) const;
    size_t  solve_y( float y, float out_t[ 2 ] ) const;
    

    float2 p[ 3 ];

};


/*
    A cubic bezier.
*/

struct cbezier
{
    cbezier();
    cbezier( const lbezier& l );
    cbezier( const qbezier& q );
    cbezier( float2 p0, float2 p1, float2 p2, float2 p3 );


    float2  evaluate( float t ) const;
    void    split( float t, cbezier out_c[ 2 ] ) const;
    qbezier derivative() const;
    bool    is_monotonic_x() const;
    bool    is_monotonic_y() const;
    size_t  solve_x( float x, float out_t[ 3 ] ) const;
    size_t  solve_y( float y, float out_t[ 3 ] ) const;
    bool    solve_self_intersection( float out_t[ 2 ] ) const;
    

    float2 p[ 4 ];

};

size_t solve_intersection
(
    const cbezier& a,
    const cbezier& b,
    std::pair< float, float > out_t[ 9 ]
);



#endif
