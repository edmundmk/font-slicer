//
//  bezier.cpp
//
//  Created by Edmund Kapusniak on 19/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#include "bezier.h"
#include "rect.h"



lbezier::lbezier()
{
}

lbezier::lbezier( float2 p0, float2 p1 )
{
    p[ 0 ] = p0;
    p[ 1 ] = p1;
}

    
float2 lbezier::evaluate( float t ) const
{
    return lerp( p[ 0 ], p[ 1 ], t );
}

void lbezier::split( float t, lbezier out_c[ 2 ] ) const
{
    float2 q = lerp( p[ 0 ], p[ 1 ], t );
    out_c[ 0 ] = lbezier( p[ 0 ], q );
    out_c[ 1 ] = lbezier( q, p[ 1 ] );
}

float2 lbezier::derivative() const
{
    return p[ 1 ] - p[ 0 ];
}

bool lbezier::is_monotonic_x() const
{
    return true;
}

bool lbezier::is_monotonic_y() const
{
    return true;
}

size_t lbezier::solve_x( float x, float* out_t ) const
{
    /*
            x = (1-t)p0 + t p1
              = p0 - tp0 + tp1
              = p0 + t( p1 - p0 )
            t = ( x - p0 ) / ( p1 - p0 )
    */
    
    float q = p[ 1 ].x - p[ 0 ].x;
    if ( q != 0 )
    {
        float t = ( x - p[ 0 ].x ) / q;
        if ( t >= 0 && t <= 1 )
        {
            *out_t = t;
            return 1;
        }
    }
    
    return 0;
}

size_t lbezier::solve_y( float y, float* out_t ) const
{
    float q = p[ 1 ].y - p[ 0 ].y;
    if ( q != 0 )
    {
        float t = ( y - p[ 0 ].y ) / q;
        if ( t >= 0 && t <= 1 )
        {
            *out_t = t;
            return 1;
        }
    }
    
    return 0;
}



qbezier::qbezier()
{
}

qbezier::qbezier( const lbezier& l )
{
    p[ 0 ] = l.p[ 0 ];
    p[ 1 ] = lerp( l.p[ 0 ], l.p[ 1 ], 0.5f );
    p[ 2 ] = l.p[ 1 ];
}

qbezier::qbezier( float2 p0, float2 p1, float2 p2 )
{
    p[ 0 ] = p0;
    p[ 1 ] = p1;
    p[ 2 ] = p2;
}

float2 qbezier::evaluate( float t ) const
{
    float2 q01 = lerp( p[ 0 ], p[ 1 ], t );
    float2 q12 = lerp( p[ 1 ], p[ 2 ], t );
    return lerp( q01, q12, t );
}

void qbezier::split( float t, qbezier out_c[ 2 ] ) const
{
    float2 q01 = lerp( p[ 0 ], p[ 1 ], t );
    float2 q12 = lerp( p[ 1 ], p[ 2 ], t );
    float2 q = lerp( q01, q12, t );
    out_c[ 0 ] = qbezier( p[ 0 ], q01, q );
    out_c[ 1 ] = qbezier( q, q12, p[ 1 ] );
}

lbezier qbezier::derivative() const
{
    return lbezier
    (
        2 * ( p[ 1 ] - p[ 0 ] ),
        2 * ( p[ 2 ] - p[ 1 ] )
    );
}

bool qbezier::is_monotonic_x() const
{
    // TODO.
    return false;
}

bool qbezier::is_monotonic_y() const
{
    // TODO.
    return false;
}

size_t qbezier::solve_x( float x, float out_t[ 2 ] ) const
{
    static const float EPSILON = 1.0e-4f;

    /*
           x = (1-t)^2 p0 + 2(1-t)t p1 + t^2 p2
           x = (1-2t+t^2)p0 + (2t-2t^2)p1 + t^2 p2
           x = (p0 - 2p1 + p2)t^2 + (-2p0 + 2p1)t + p0
           
           t = ( -b +- sqrt( b^2 - 4ac ) ) / 2a
                a = p0 - 2p1 + p2
                b = -2p0 + 2p1
                c = p0 - x
    */
    
    size_t i = 0;

    float a = p[ 0 ].x - 2 * p[ 1 ].x + p[ 2 ].x;
    float b = -2 * p[ 0 ].x + 2 * p[ 1 ].x;
    float c = p[ 0 ]. x - x;
    
    if ( fabsf( a ) < EPSILON )
    {
        /*
            bt + c = 0
        */
        
        out_t[ 0 ] = -c / b;
        return 1;
    }
    
    float d = b * b - 4 * a * c;
    if ( d >= 0 )
    {
        d = sqrtf( d );
        float t0 = ( -b - d ) / ( 2 * a );
        float t1 = ( -b + d ) / ( 2 * a );
        
        if ( t0 >= 0 && t0 <= 1 )
        {
            out_t[ i ] = t0;
            i += 1;
        }
        if ( t1 >= 0 && t1 <= 1 )
        {
            out_t[ i ] = t1;
            i += 1;
        }
    }
    
    return 0;
}

size_t qbezier::solve_y( float y, float out_t[ 2 ] ) const
{
    static const float EPSILON = 1.0e-4f;

    size_t i = 0;

    float a = p[ 0 ].y - 2 * p[ 1 ].y + p[ 2 ].y;
    float b = -2 * p[ 0 ].y + 2 * p[ 1 ].y;
    float c = p[ 0 ]. y - y;
    
    if ( fabsf( a ) < EPSILON )
    {
        /*
            bt + c = 0
        */
        
        out_t[ 0 ] = -c / b;
        return 1;
    }
    
    float d = b * b - 4 * a * c;
    if ( d >= 0 )
    {
        d = sqrtf( d );
        float t0 = ( -b - d ) / ( 2 * a );
        float t1 = ( -b + d ) / ( 2 * a );
        
        if ( t0 >= 0 && t0 <= 1 )
        {
            out_t[ i ] = t0;
            i += 1;
        }
        if ( t1 >= 0 && t1 <= 1 )
        {
            out_t[ i ] = t1;
            i += 1;
        }
    }
    
    return i;
}





cbezier::cbezier()
{
}

cbezier::cbezier( const lbezier& l )
{
    p[ 0 ] = l.p[ 0 ];
    p[ 1 ] = lerp( l.p[ 0 ], l.p[ 1 ], 1.0f / 3.0f );
    p[ 2 ] = lerp( l.p[ 0 ], l.p[ 1 ], 2.0f / 3.0f );
    p[ 3 ] = l.p[ 1 ];
}

cbezier::cbezier( const qbezier& c )
{
    p[ 0 ] = c.p[ 0 ];
    p[ 1 ] = c.p[ 0 ] + ( 2.0f / 3.0f ) * ( c.p[ 1 ] - c.p[ 0 ] );
    p[ 2 ] = c.p[ 2 ] + ( 2.0f / 3.0f ) * ( c.p[ 1 ] - c.p[ 2 ] );
    p[ 3 ] = c.p[ 2 ];
}

cbezier::cbezier( float2 p0, float2 p1, float2 p2, float2 p3 )
{
    p[ 0 ] = p0;
    p[ 1 ] = p1;
    p[ 2 ] = p2;
    p[ 3 ] = p3;
}

float2 cbezier::evaluate( float t ) const
{
    float2 q01 = lerp( p[ 0 ], p[ 1 ], t );
    float2 q12 = lerp( p[ 1 ], p[ 2 ], t );
    float2 q23 = lerp( p[ 2 ], p[ 3 ], t );
    float2 q012 = lerp( q01, q12, t );
    float2 q123 = lerp( q12, q23, t );
    return lerp( q012, q123, t );
}

void cbezier::split( float t, cbezier out_c[ 2 ] ) const
{
    float2 q01 = lerp( p[ 0 ], p[ 1 ], t );
    float2 q12 = lerp( p[ 1 ], p[ 2 ], t );
    float2 q23 = lerp( p[ 2 ], p[ 3 ], t );
    float2 q012 = lerp( q01, q12, t );
    float2 q123 = lerp( q12, q23, t );
    float2 q = lerp( q012, q123, t );
    out_c[ 0 ] = cbezier( p[ 0 ], q01, q012, q );
    out_c[ 1 ] = cbezier( q, q123, q23, p[ 3 ] );
}

qbezier cbezier::derivative() const
{
    return qbezier
    (
        3 * ( p[ 1 ] - p[ 0 ] ),
        3 * ( p[ 2 ] - p[ 1 ] ),
        3 * ( p[ 3 ] - p[ 2 ] )
    );
}


static bool is_monotonic( float f, float g )
{
    if ( f < 0 || g < 0 )
    {
        // Outside first quadrant.
        return false;
    }
    
    if ( g <= ( 2 / 3 ) - f )
    {
        // In bottom left area A/B/C.
        return true;
    }
    
    if ( g <= 1 - 2 * f )
    {
        // In left area B.
        return true;
    }
    
    if ( g <= 0.5f - 0.5f * f )
    {
        // In right area C.
        return true;
    }

    /*
        Ellipse condition:
        
        1 - .5f - .5sqrt(-3f^2 + 4f) < g < 1 - .5f + .5sqrt(-3f^2 + 4f)
                        (2g + f - 2)^2 < -3f^2 + 4f
    */
    float lhs = 2 * g + f - 2;
    if ( lhs * lhs <= -3 * f * f + 4 * f )
    {
        // In ellipse D.
        return true;
    }

    return false;
}

bool cbezier::is_monotonic_x() const
{
    float f = ( p[ 1 ].x - p[ 0 ].x ) / ( p[ 3 ].x - p[ 0 ].x );
    float g = ( p[ 3 ].x - p[ 2 ].x ) / ( p[ 3 ].x - p[ 0 ].x );
    return is_monotonic( f, g );
}

bool cbezier::is_monotonic_y() const
{
    float f = ( p[ 1 ].y - p[ 0 ].y ) / ( p[ 3 ].y - p[ 0 ].y );
    float g = ( p[ 3 ].y - p[ 2 ].y ) / ( p[ 3 ].y - p[ 0 ].y );
    return is_monotonic( f, g );
}


static size_t solve( float f, float g, float v, float out_t[ 3 ] )
{
    static const float EPSILON = 1.0e-4f;

    size_t i = 0;

    float d = 3 * f + 3 * g - 2;
    float n = 2 * f + g - 1;
    
    if ( fabsf( d ) < EPSILON )
    {
        if ( fabsf( n ) < EPSILON )
        {
            // Curve is linear.
            //
            // 3ft - x = 0
            //
            
            out_t[ 0 ] = v / ( 3 * f );
            return 1;
        }
        else
        {
            // Curve is quadratic.
            //
            // -3nt^2 + 3ft - x = 0
            //
            
            float a = -3.0f * n;
            float b = 3.0f * f;
            float c = -v;
            
            float disc = b * b - 4 * a * c;
            float sqrt_disc = sqrtf( disc );
            float t0 = ( -b + sqrt_disc ) / ( 2.0f * a );
            float t1 = ( -b - sqrt_disc ) / ( 2.0f * a );
            
            if ( t0 >= 0 && t0 <= 1 )
            {
                out_t[ i ] = t0;
                i += 1;
            }
            if ( t1 >= 0 && t1 <= 1 )
            {
                out_t[ i ] = t1;
                i += 1;
            }
            
            return i;
        }
    }
    
    
    float r = ( n * n - f * d ) / ( d * d );
    float q = ( 3 * f * d * n - 2 * n * n * n ) / ( d * d * d ) - ( v / d );
    float disc = q * q - 4 * r * r * r;
    
    if ( disc > 0 )
    {
        float w3; // always pick nonzero solution for w^3.
        if ( q > 0 )
            w3 = ( -q - sqrtf( disc ) ) * 0.5f;
        else
            w3 = ( -q + sqrtf( disc ) ) * 0.5f;
        float w = cbrtf( w3 );
        float u = w + r / w;
        float t = u + n / d;
        
        if ( t >= 0 && t <= 1 )
        {
            out_t[ i ] = t;
            i += 1;
        }
    }
    else
    {
        float theta = acosf( -q / ( 2 * sqrtf( r * r * r ) ) );
        float phi0 = theta / 3;
        float phi1 = ( theta + F_TAU ) / 3;
        float phi2 = ( theta + 2 * F_TAU ) / 3;

        float sqrt_r = sqrtf( r );
        float n_over_d = n / d;

        float t0 = 2 * sqrt_r * cosf( phi0 ) + n_over_d;
        float t1 = 2 * sqrt_r * cosf( phi1 ) + n_over_d;
        float t2 = 2 * sqrt_r * cosf( phi2 ) + n_over_d;
        
        if ( t0 >= 0 && t0 <= 1 )
        {
            out_t[ i ] = t0;
            i += 1;
        }
        if ( t1 >= 0 && t1 <= 1 )
        {
            out_t[ i ] = t1;
            i += 1;
        }
        if ( t2 >= 0 && t2 <= 2 )
        {
            out_t[ i ] = t2;
            i += 1;
        }
    }

    return i;
}


size_t cbezier::solve_x( float x, float out_t[ 3 ] ) const
{
    float f = ( p[ 1 ].x - p[ 0 ].x ) / ( p[ 3 ].x - p[ 0 ].x );
    float g = ( p[ 3 ].x - p[ 2 ].x ) / ( p[ 3 ].x - p[ 0 ].x );
    float v = ( x - p[ 0 ].x ) / ( p[ 3 ].x - p[ 0 ].x );
    return solve( f, g, v, out_t );
}

size_t cbezier::solve_y( float y, float out_t[ 3 ] ) const
{
    float f = ( p[ 1 ].y - p[ 0 ].y ) / ( p[ 3 ].y - p[ 0 ].y );
    float g = ( p[ 3 ].y - p[ 2 ].y ) / ( p[ 3 ].y - p[ 0 ].y );
    float v = ( y - p[ 0 ].y ) / ( p[ 3 ].y - p[ 0 ].y );
    return solve( f, g, v, out_t );
}

bool cbezier::solve_self_intersection( float out_t[ 2 ] ) const
{
    qbezier h = derivative();

    float G0 = h.p[ 0 ].x;
    float Gl = h.p[ 1 ].x - h.p[ 0 ].x;
    float Gm = h.p[ 2 ].x - 2 * h.p[ 1 ].x + h.p[ 0 ].x;
    
    float H0 = h.p[ 0 ].y;
    float Hl = h.p[ 1 ].y - h.p[ 0 ].y;
    float Hm = h.p[ 2 ].y - 2 * h.p[ 1 ].y + h.p[ 0 ].y;
    
    if ( Gm == 0 || Hm == 0 )
    {
        // Curve is quadratic in x or y.  This can still lead to
        // self-intersection but I haven't done the maths (hopefully simpler).
        // If B(t) is quadratic then B(s) - B(t) is a degenerate rectangular
        // hyperbola meeting in the middle (two lines) rather than a degenerate
        // cubic (line and ellipse).  Effectively the ellipse has infinite
        // radius and opens into a line.  This line can still intersect the
        // ellipse from a cubic in x or y.
        return false;
    }
    
    float u = ( H0 / Hm - G0 / Gm ) / ( 2 * ( Gl / Gm - Hl / Hm ) );
    float vsq = -3 * u * u - 6 * ( Hl / Hm ) * u - 3 * H0 / Hm;
    if ( vsq < 0 )
        return false;
    
    float v = sqrtf( vsq );
    
    out_t[ 0 ] = u - v;
    out_t[ 1 ] = u + v;
    return true;
}



/*
    From Graphics Gems 4.  Intersecting Parametric Cubic Curves by Midpoint
    Subdivision, R. Viktor Klassen.
*/


static const float INV_EPS = (float)( 1 << 14 );
inline float log4f( float x ) { return 0.5f * log2f( x ); }


static rect calculate_bbox( const cbezier& c )
{
    rect bbox;
    for ( size_t i = 0; i < 4; ++i )
        bbox = bbox.expand( c.p[ i ] );
    return bbox;
}


static bool intersect_bbox( const cbezier& a, const cbezier& b )
{
    rect a_bbox = calculate_bbox( a );
    rect b_bbox = calculate_bbox( b );
    if (    a_bbox.maxx < b_bbox.minx
         || a_bbox.maxy < b_bbox.miny
         || a_bbox.minx > b_bbox.maxx
         || a_bbox.miny > b_bbox.maxy )
    {
        return false;
    }
    else
    {
        return true;
    }
}


static int estimate_subdivision_depth( const cbezier& c )
{
    float2 l1 = abs( ( c.p[ 2 ] - c.p[ 1 ] ) - ( c.p[ 1 ] - c.p[ 0 ] ) );
    float2 l2 = abs( ( c.p[ 3 ] - c.p[ 2 ] ) - ( c.p[ 2 ] - c.p[ 1 ] ) );
    float2 l = max( l1, l2 );
    float l0 = max( l.x, l.y );
    if ( l0 * 0.75f * F_SQRT2 + 1.0f != 1.0f )
        return (int)ceilf( log4f( F_SQRT2 * 6.0f / 8.0f * INV_EPS * 10.0f ) );
    else
        return 0;
}


static void recursively_intersect
(
    const cbezier& a,
    float t0,
    float t1,
    int deptha,
    const cbezier& b,
    float u0,
    float u1,
    int depthb,
    std::pair< float, float > out_t[ 9 ],
    size_t& index
)
{
    if ( deptha > 0 )
    {
        cbezier A[ 2 ];
        a.split( 0.5f, A );
        float tmid = ( t0 + t1 ) * 0.5f;
        deptha -= 1;

        if ( depthb > 0 )
        {
            cbezier B[ 2 ];
            b.split( 0.5f, B );
            float umid = ( u0 + u1 ) * 0.5f;
            depthb -= 1;
            
            if ( intersect_bbox( A[ 0 ], B[ 0 ] ) )
                recursively_intersect( A[ 0 ], t0, tmid, deptha,
                                       B[ 0 ], u0, umid, depthb,
                                       out_t, index );
            
            if ( intersect_bbox( A[ 0 ], B[ 1 ] ) )
                recursively_intersect( A[ 0 ], t0, tmid, deptha,
                                       B[ 1 ], umid, u1, depthb,
                                       out_t, index );

            if ( intersect_bbox( A[ 1 ], B[ 0 ] ) )
                recursively_intersect( A[ 1 ], tmid, t1, deptha,
                                       B[ 0 ], u0, umid, depthb,
                                       out_t, index );

            if ( intersect_bbox( A[ 1 ], B[ 1 ] ) )
                recursively_intersect( A[ 1 ], tmid, t1, deptha,
                                       B[ 1 ], umid, u1, depthb,
                                       out_t, index );
        }
        else
        {
            if ( intersect_bbox( A[ 0 ], b ) )
                recursively_intersect( A[ 0 ], t0, tmid, deptha,
                                       b, u0, u1, depthb,
                                       out_t, index );

            if ( intersect_bbox( A[ 1 ], b ) )
                recursively_intersect( A[ 1 ], tmid, t1, deptha,
                                       b, u0, u1, depthb,
                                       out_t, index );
        }
    }
    else
    {
        if ( depthb > 0 )
        {
            cbezier B[ 2 ];
            b.split( 0.5f, B );
            float umid = ( u0 + u1 ) * 0.5f;
            depthb -= 1;

            if ( intersect_bbox( a, B[ 0 ] ) )
                recursively_intersect( a, t0, t1, deptha,
                                       B[ 0 ], u0, umid, depthb,
                                       out_t, index );

            if ( intersect_bbox( a, B[ 1 ] ) )
                recursively_intersect( a, t0, t1, deptha,
                                       B[ 1 ], umid, u1, depthb,
                                       out_t, index );
        }
        else
        {
            // Both segments are fully subdivided, do line segments.
            float2 lk = a.p[ 3 ] - a.p[ 0 ];
            float2 nm = b.p[ 3 ] - b.p[ 0 ];
            float2 mk = b.p[ 0 ] - a.p[ 0 ];
            float det = nm.x * lk.y - nm.y * lk.x;
            if ( 1.0f + det == 1.0f )
                return;
        
            float detinv = 1.0f / det;
            float s = ( nm.x * mk.y - nm.y * mk.x ) * detinv;
            float t = ( lk.x * mk.y - lk.y * mk.x ) * detinv;
            if ( s < 0.0f || s > 1.0f || t < 0.0f || t > 1.0f )
                return;
            
            assert( index < 9 );
            out_t[ index ] = std::make_pair
            (
                lerp( t0, t1, s ),
                lerp( u0, u1, t )
            );
            index += 1;
        }
    }
}


size_t solve_intersection
(
    const cbezier& a,
    const cbezier& b,
    std::pair< float, float > out_t[ 9 ]
)
{
    size_t index = 0;
    if ( intersect_bbox( a, b ) )
    {
        int ra = estimate_subdivision_depth( a );
        int rb = estimate_subdivision_depth( b );
        recursively_intersect( a, 0.0f, 1.0f, ra, b, 0.0f, 1.0f, rb, out_t, index );
    }
    return index;
}






