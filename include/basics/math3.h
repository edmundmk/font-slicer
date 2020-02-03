//
//  math3.h
//
//  Created by Edmund Kapusniak on 02/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef MATH3_H
#define MATH3_H


#include <assert.h>
#include <math.h>
#include <limits>


/*
    New version of math3.  Stuck with float3 rather than vec3, as it is nicer
    and HLSL, Cg, OpenCL, and CUDA all use it, leaving GLSL as the odd one out.
*/


static const float F_INFINITY   = std::numeric_limits< float >::infinity();
static const float F_NAN        = std::numeric_limits< float >::quiet_NaN();
static const float F_TAU        = 6.283185307179586f;
static const float F_PI         = F_TAU * 0.5f;
static const float F_SQRT2      = sqrtf( 2.0f );


struct float2
{

    float2();
    float2( float x, float y );

    union { struct
    {
        float x;
        float y;
    };
        float v[ 2 ];
    };

    float& operator [] ( size_t index );
    float operator [] ( size_t index ) const;

};


struct float3
{

    float3();
    float3( float x, float y, float z );
    float3( float2 xy, float z );

    union { struct
    {
        float x;
        float y;
        float z;
    };
        float v[ 3 ];
    };

    float& operator [] ( size_t index );
    float operator [] ( size_t index ) const;

    float2 xy() const;

};


struct float4
{

    float4();
    float4( float x, float y, float z, float w );
    float4( float2 xy, float z, float w );
    float4( float3 xyz, float w );

    union { struct
    {
        float x;
        float y;
        float z;
        float w;
    };
        float v[ 4 ];
    };

    float& operator [] ( size_t index );
    float operator [] ( size_t index ) const;

    float3 xyz();
    float2 xy();

};



struct matrix2
{

    matrix2();
    matrix2
    (
        float m00, float m10,
        float m01, float m11
    );

    float2 m[ 2 ];

    float2& operator [] ( size_t index );
    float2 operator [] ( size_t index ) const;

};


struct matrix3
{

    matrix3();
    matrix3
    (
        float m00, float m10, float m20,
        float m01, float m11, float m21,
        float m02, float m12, float m22
    );

    float3 m[ 3 ];

    float3& operator [] ( size_t index );
    float3 operator [] ( size_t index ) const;

};


struct matrix4
{

    matrix4();
    matrix4
    (
        float m00, float m10, float m20, float m30,
        float m01, float m11, float m21, float m31,
        float m02, float m12, float m22, float m32,
        float m03, float m13, float m23, float m33
    );

    float4 m[ 4 ];

    float4& operator [] ( size_t index );
    float4 operator [] ( size_t index ) const;

};




inline float min( float a, float b ) { return a < b ? a : b; }
inline float max( float a, float b ) { return a > b ? a : b; }
inline float clamp( float v, float min, float max ) { return v < min ? min : v > max ? max : v; }
inline float lerp( float a, float b, float t ) { return a + ( b - a ) * t; }


inline float2::float2() : x( 0.0f ), y( 0.0f ) {}
inline float2::float2( float x, float y ) : x( x ), y( y ) {}
inline float& float2::operator [] ( size_t index ) { return v[ index ]; }
inline float float2::operator [] ( size_t index ) const { return v[ index ]; }

inline float2 operator + ( float2 a ) { return a; }
inline float2 operator - ( float2 a ) { return float2( -a.x, -a.y ); }
inline float2 operator + ( float2 a, float2 b ) { return float2( a.x + b.x, a.y + b.y ); }
inline float2 operator - ( float2 a, float2 b ) { return float2( a.x - b.x, a.y - b.y ); }
inline float2 operator * ( float2 a, float2 b ) { return float2( a.x * b.x, a.y * b.y ); }
inline float2 operator * ( float2 a, float b ) { return float2( a.x * b, a.y * b ); }
inline float2 operator * ( float a, float2 b ) { return float2( a * b.x, a * b.y ); }
inline float2 operator / ( float2 a, float2 b ) { return float2( a.x / b.x, a.y / b.y ); }
inline float2 operator / ( float2 a, float b ) { return float2( a.x / b, a.y / b ); }
inline float2 operator / ( float a, float2 b ) { return float2( a / b.x, a / b.y ); }
inline float2& operator += ( float2& a, float2 b ) { return a = a + b; }
inline float2& operator -= ( float2& a, float2 b ) { return a = a - b; }
inline float2& operator *= ( float2& a, float2 b ) { return a = a * b; }
inline float2& operator *= ( float2& a, float b ) { return a = a * b; }
inline float2& operator /= ( float2& a, float2 b ) { return a = a / b; }
inline float2& operator /= ( float2& a, float b ) { return a = a / b; }

inline float2 min( float2 a, float2 b ) { return float2( min( a.x, b.x ), min( a.y, b.y ) ); }
inline float2 max( float2 a, float2 b ) { return float2( max( a.x, b.x ), max( a.y, b.y ) ); }
inline float2 abs( float2 v ) { return float2( fabsf( v.x ), fabsf( v.y ) ); }
inline float2 lerp( float2 a, float2 b, float t ) { return float2( lerp( a.x, b.x, t ), lerp( a.y, b.y, t ) ); }
inline float dot( float2 a, float2 b ) { return a.x * b.x + a.y * b.y; }
inline float lengthsq( float2 v ) { return dot( v, v ); }
inline float length( float2 v ) { return sqrt( lengthsq( v ) ); }
inline float2 normalize( float2 v ) { return v / length( v ); }


inline float3::float3() : x( 0.0f ), y( 0.0f ), z( 0.0f ) {}
inline float3::float3( float x, float y, float z ) : x( x ), y( y ), z( z ) {}
inline float3::float3( float2 xy, float z ) : x( xy.x ), y( xy.y ), z( z ) {}
inline float& float3::operator [] ( size_t index ) { return v[ index ]; }
inline float float3::operator [] ( size_t index ) const { return v[ index ]; }

inline float3 operator + ( float3 a ) { return a; }
inline float3 operator - ( float3 a ) { return float3( -a.x, -a.y, -a.z ); }
inline float3 operator + ( float3 a, float3 b ) { return float3( a.x + b.x, a.y + b.y, a.z + b.z ); }
inline float3 operator - ( float3 a, float3 b ) { return float3( a.x - b.x, a.y - b.y, a.z - b.z ); }
inline float3 operator * ( float3 a, float3 b ) { return float3( a.x * b.x, a.y * b.y, a.z * b.z ); }
inline float3 operator * ( float3 a, float b ) { return float3( a.x * b, a.y * b, a.z * b ); }
inline float3 operator * ( float a, float3 b ) { return float3( a * b.x, a * b.y, a * b.z ); }
inline float3 operator / ( float3 a, float3 b ) { return float3( a.x / b.x, a.y / b.y, a.z / b.z ); }
inline float3 operator / ( float3 a, float b ) { return float3( a.x / b, a.y / b, a.z / b ); }
inline float3 operator / ( float a, float3 b ) { return float3( a / b.x, a / b.y, a / b.z ); }
inline float3& operator += ( float3& a, float3 b ) { return a = a + b; }
inline float3& operator -= ( float3& a, float3 b ) { return a = a - b; }
inline float3& operator *= ( float3& a, float3 b ) { return a = a * b; }
inline float3& operator *= ( float3& a, float b ) { return a = a * b; }
inline float3& operator /= ( float3& a, float3 b ) { return a = a / b; }
inline float3& operator /= ( float3& a, float b ) { return a = a / b; }

inline float3 lerp( float3 a, float3 b, float t ) { return float3( lerp( a.x, b.x, t ), lerp( a.y, b.y, t ), lerp( a.z, b.z, t ) ); }
inline float dot( float3 a, float3 b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float lengthsq( float3 v ) { return dot( v, v ); }
inline float length( float3 v ) { return sqrt( lengthsq( v ) ); }
inline float3 normalize( float3 v ) { return v / length( v ); }


inline float4::float4() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f ) {}
inline float4::float4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}
inline float4::float4( float2 xy, float z, float w ) : x( xy.x ), y( xy.y ), z( z ), w( w ) {}
inline float4::float4( float3 xyz, float w ) : x( xyz.x ), y( xyz.y ), z( xyz.z ), w( w ) {}

inline float4 operator + ( float4 a ) { return a; }
inline float4 operator - ( float4 a ) { return float4( -a.x, -a.y, -a.z, -a.w ); }
inline float4 operator + ( float4 a, float4 b ) { return float4( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w ); }
inline float4 operator - ( float4 a, float4 b ) { return float4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w ); }
inline float4 operator * ( float4 a, float4 b ) { return float4( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w ); }
inline float4 operator * ( float4 a, float b ) { return float4( a.x * b, a.y * b, a.z * b, a.w * b ); }
inline float4 operator * ( float a, float4 b ) { return float4( a * b.x, a * b.y, a * b.z, a * b.w ); }
inline float4 operator / ( float4 a, float4 b ) { return float4( a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w ); }
inline float4 operator / ( float4 a, float b ) { return float4( a.x / b, a.y / b, a.z / b, a.w / b ); }
inline float4 operator / ( float a, float4 b ) { return float4( a / b.x, a / b.y, a / b.z, a / b.w ); }
inline float4& operator += ( float4& a, float4 b ) { return a = a + b; }
inline float4& operator -= ( float4& a, float4 b ) { return a = a - b; }
inline float4& operator *= ( float4& a, float4 b ) { return a = a * b; }
inline float4& operator *= ( float4& a, float b ) { return a = a * b; }
inline float4& operator /= ( float4& a, float4 b ) { return a = a / b; }
inline float4& operator /= ( float4& a, float b ) { return a = a / b; }

inline float4 lerp( float4 a, float4 b, float t ) { return float4( lerp( a.x, b.x, t ), lerp( a.y, b.y, t ), lerp( a.z, b.z, t ), lerp( a.w, b.w, t ) ); }
inline float dot( float4 a, float4 b ) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
inline float lengthsq( float4 v ) { return dot( v, v ); }
inline float length( float4 v ) { return sqrt( lengthsq( v ) ); }
inline float4 normalize( float4 v ) { return v / length( v ); }



inline float3 cross( float3 a, float3 b )
{
    return float3
    (
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}



inline float2 slerp( float2 a, float2 b, float t )
{
    /*
        We expect a and b to be normalised.  From Shoemake:

        cos Z = a . b ; ( sin [ ( 1 - t )Z ] a + sin [ tZ ] b ) / sin Z

        But we can break sin [ ( 1 - t )Z ] into:

            sin [ Z - tZ ]
            sin Z cos tZ - cos Z sin tZ

        Giving:

            ( sin Z cos [ tZ ] a - cos Z sin [ tZ ] a + sin [ tZ ] b ) / sin Z
            ( sin Z cos [ tZ ] a + sin [ tZ ] b - cos Z sin [ tZ ] a ) / sin Z
            cos [ tZ ] a + ( sin [ tZ ] b - cos Z sin [ tZ ] a ) / sin Z
            cos [ tZ ] a + ( sin [ tZ ] / sin Z )( b - cos Z a )
            cos [ tZ ] a + ( sin [ tZ ] / sin Z )( b - ( a . b )a )
    */

    float cos_omega = dot( a, b );
    float omega = acosf( cos_omega );
    float t_omega = t * omega;
    float2 v = b - cos_omega * a;
    return cosf( t_omega ) * a - ( sinf( t_omega ) / sinf( omega ) ) * v;
}

inline float2 slerp_step( float2 a, float2 b, float t, float* out_step )
{
    /*
        Finds the slerp step parameter to step between a and b in steps of
        t.  This is 2 * a . b.
    */

    float2 next = slerp( a, b, t );
    *out_step = 2.0f * dot( a, next );
    return next;
}

inline float2 slerp_next( float2 a, float2 b, float step )
{
    /*
        Expects a and b to be normalized.  Finds c such that it lies along
        the great circle between a and b, and is the same angle from b as b
        is from a.
    */

    return step * b - a;
}


inline float3 slerp( float3 a, float3 b, float t )
{
    float cos_omega = dot( a, b );
    float omega = acosf( cos_omega );
    float t_omega = t * omega;
    float3 v = b - cos_omega * a;
    return cosf( t_omega ) * a + ( sinf( t_omega ) / sinf( omega ) ) * v;
}

inline float3 slerp_step( float3 a, float3 b, float t, float* out_step )
{
    float3 next = slerp( a, b, t );
    *out_step = 2.0f * dot( a, next );
    return next;
}

inline float3 slerp_next( float3 a, float3 b, float step )
{
    return step * b - a;
}




inline matrix2::matrix2()
{
    m[ 0 ] = float2( 1.0f, 0.0f );
    m[ 1 ] = float2( 0.0f, 1.0f );
}

inline matrix2::matrix2(
            float m00, float m10,
                float m01, float m11 )
{
    m[ 0 ] = float2( m00, m01 );
    m[ 1 ] = float2( m10, m11 );
}

inline float2& matrix2::operator [] ( size_t index )
{
    return m[ index ];
}

inline float2 matrix2::operator [] ( size_t index ) const
{
    return m[ index ];
}

inline matrix2 operator * ( const matrix2& a, const matrix2& b )
{
    float2 row0 = float2( a[ 0 ][ 0 ], a[ 1 ][ 0 ] );
    float2 row1 = float2( a[ 0 ][ 1 ], a[ 1 ][ 1 ] );
    return matrix2
    (
        dot( row0, b[ 0 ] ), dot( row0, b[ 1 ] ),
        dot( row1, b[ 0 ] ), dot( row1, b[ 1 ] )
    );
}

inline float2 operator * ( float2 a, const matrix2& b )
{
    return float2( dot( a, b[ 0 ] ), dot( a, b[ 1 ] ) );
}

inline float2 operator * ( const matrix2& a, float2 b )
{
    float2 row0 = float2( a[ 0 ][ 0 ], a[ 1 ][ 0 ] );
    float2 row1 = float2( a[ 0 ][ 1 ], a[ 1 ][ 1 ] );
    return float2( dot( row0, b ), dot( row1, b ) );
}

inline matrix2& operator *= ( matrix2& a, const matrix2& b )
{
    a = a * b;
    return a;
}

inline float2& operator *= ( float2& a, const matrix2& b )
{
    a = a * b;
    return a;
}

inline matrix2 inverse( const matrix2& m )
{
    float a = m[ 0 ][ 0 ];
    float b = m[ 1 ][ 0 ];
    float c = m[ 0 ][ 1 ];
    float d = m[ 1 ][ 1 ];
    float det = 1.0f / ( a * d - b * c );
    return matrix2
    (
         d * det, -b * det,
        -c * det,  a * det
    );
}



inline matrix3::matrix3()
{
    m[ 0 ] = float3( 1.0f, 0.0f, 0.0f );
    m[ 1 ] = float3( 0.0f, 1.0f, 0.0f );
    m[ 2 ] = float3( 0.0f, 0.0f, 1.0f );
}

inline matrix3::matrix3(
            float m00, float m10, float m20,
                float m01, float m11, float m21,
                    float m02, float m12, float m22 )
{
    m[ 0 ] = float3( m00, m01, m02 );
    m[ 1 ] = float3( m10, m11, m12 );
    m[ 2 ] = float3( m20, m21, m22 );
}

inline float3& matrix3::operator [] ( size_t index )
{
    return m[ index ];
}

inline float3 matrix3::operator [] ( size_t index ) const
{
    return m[ index ];
}

inline matrix3 operator * ( const matrix3& a, const matrix3& b )
{
    float3 row0 = float3( a[ 0 ][ 0 ], a[ 1 ][ 0 ], a[ 2 ][ 0 ] );
    float3 row1 = float3( a[ 0 ][ 1 ], a[ 1 ][ 1 ], a[ 2 ][ 1 ] );
    float3 row2 = float3( a[ 0 ][ 2 ], a[ 1 ][ 2 ], a[ 2 ][ 2 ] );
    return matrix3
    (
        dot( row0, b[ 0 ] ), dot( row0, b[ 1 ] ), dot( row0, b[ 2 ] ),
        dot( row1, b[ 0 ] ), dot( row1, b[ 1 ] ), dot( row1, b[ 2 ] ),
        dot( row2, b[ 0 ] ), dot( row2, b[ 1 ] ), dot( row2, b[ 2 ] )
    );
}

inline float3 operator * ( float3 a, const matrix3& b )
{
    return float3( dot( a, b[ 0 ] ), dot( a, b[ 1 ] ), dot( a, b[ 2 ] ) );
}

inline float3 operator * ( const matrix3& a, float3 b )
{
    float3 row0 = float3( a[ 0 ][ 0 ], a[ 1 ][ 0 ], a[ 2 ][ 0 ] );
    float3 row1 = float3( a[ 0 ][ 1 ], a[ 1 ][ 1 ], a[ 2 ][ 1 ] );
    float3 row2 = float3( a[ 0 ][ 2 ], a[ 1 ][ 2 ], a[ 2 ][ 2 ] );
    return float3( dot( row0, b ), dot( row1, b ), dot( row2, b ) );
}

inline matrix3& operator *= ( matrix3& a, const matrix3& b )
{
    a = a * b;
    return a;
}

inline float3& operator *= ( float3& a, const matrix3& b )
{
    a = a * b;
    return a;
}

inline matrix3 inverse( const matrix3& m )
{
    if ( m[ 2 ][ 0 ] == 0.0f && m[ 2 ][ 1 ] == 0.0f && m[ 2 ][ 2 ] == 1.0f )
    {
        // This is a 2D transformation matrix.  It can be decomposed into
        // a rotation part R and a translation part T.  M = R * T.  Therefore
        // the inverse M' = T' * R', since M * M' = R * T * T' * R' = I.

        matrix2 r = matrix2
        (
            m[ 0 ][ 0 ], m[ 1 ][ 0 ],
            m[ 0 ][ 1 ], m[ 1 ][ 1 ]
        );
        matrix2 r_inverse = inverse( r );
        float2 t_inverse = -float2( m[ 0 ][ 2 ], m[ 1 ][ 2 ] );


        // And instead of doing a full 3x3 multiply to combine T' * R', we
        // can exploit the fact that a lot of the elements are 0.

        float a = r_inverse[ 0 ][ 0 ];
        float b = r_inverse[ 1 ][ 0 ];
        float c = r_inverse[ 0 ][ 1 ];
        float d = r_inverse[ 1 ][ 1 ];
        float p = a * t_inverse.x + c * t_inverse.y;
        float q = b * t_inverse.x + d * t_inverse.y;

        return matrix3
        (
            a, b, 0.0f,
            c, d, 0.0f,
            p, q, 1.0f
        );
    }
    else
    {
        assert( ! "general matrix3 inverse not implemented" );
        return matrix3();
    }
}



inline matrix4::matrix4()
{
    m[ 0 ] = float4( 1.0f, 0.0f, 0.0f, 0.0f );
    m[ 1 ] = float4( 0.0f, 1.0f, 0.0f, 0.0f );
    m[ 2 ] = float4( 0.0f, 0.0f, 1.0f, 0.0f );
    m[ 3 ] = float4( 0.0f, 0.0f, 0.0f, 1.0f );
}

inline matrix4::matrix4(
            float m00, float m10, float m20, float m30,
                float m01, float m11, float m21, float m31,
                    float m02, float m12, float m22, float m32,
                        float m03, float m13, float m23, float m33 )
{
    m[ 0 ] = float4( m00, m01, m02, m03 );
    m[ 1 ] = float4( m10, m11, m12, m13 );
    m[ 2 ] = float4( m20, m21, m22, m23 );
    m[ 3 ] = float4( m30, m31, m32, m33 );
}

inline float4& matrix4::operator [] ( size_t index )
{
    return m[ index ];
}

inline float4 matrix4::operator [] ( size_t index ) const
{
    return m[ index ];
}

inline matrix4 operator * ( const matrix4& a, const matrix4& b )
{
    float4 row0 = float4( a[ 0 ][ 0 ], a[ 1 ][ 0 ], a[ 2 ][ 0 ], a[ 3 ][ 0 ] );
    float4 row1 = float4( a[ 0 ][ 1 ], a[ 1 ][ 1 ], a[ 2 ][ 1 ], a[ 3 ][ 1 ] );
    float4 row2 = float4( a[ 0 ][ 2 ], a[ 1 ][ 2 ], a[ 2 ][ 2 ], a[ 3 ][ 2 ] );
    float4 row3 = float4( a[ 0 ][ 3 ], a[ 1 ][ 3 ], a[ 2 ][ 3 ], a[ 3 ][ 3 ] );
    return matrix4
    (
        dot( row0, b[ 0 ] ), dot( row0, b[ 1 ] ), dot( row0, b[ 2 ] ), dot( row0, b[ 3 ] ),
        dot( row1, b[ 0 ] ), dot( row1, b[ 1 ] ), dot( row1, b[ 2 ] ), dot( row1, b[ 3 ] ),
        dot( row2, b[ 0 ] ), dot( row2, b[ 1 ] ), dot( row2, b[ 2 ] ), dot( row2, b[ 3 ] ),
        dot( row3, b[ 0 ] ), dot( row3, b[ 1 ] ), dot( row3, b[ 2 ] ), dot( row3, b[ 3 ] )
    );
}

inline float4 operator * ( float4 a, const matrix4& b )
{
    return float4( dot( a, b[ 0 ] ), dot( a, b[ 1 ] ), dot( a, b[ 2 ] ), dot ( a, b[ 3 ] ) );
}

inline float4 operator * ( const matrix4& a, float4 b )
{
    float4 row0 = float4( a[ 0 ][ 0 ], a[ 1 ][ 0 ], a[ 2 ][ 0 ], a[ 3 ][ 0 ] );
    float4 row1 = float4( a[ 0 ][ 1 ], a[ 1 ][ 1 ], a[ 2 ][ 1 ], a[ 3 ][ 1 ] );
    float4 row2 = float4( a[ 0 ][ 2 ], a[ 1 ][ 2 ], a[ 2 ][ 2 ], a[ 3 ][ 2 ] );
    float4 row3 = float4( a[ 0 ][ 3 ], a[ 1 ][ 3 ], a[ 2 ][ 3 ], a[ 3 ][ 3 ] );
    return float4( dot( row0, b ), dot( row1, b ), dot( row2, b ), dot( row3, b ) );
}

inline matrix4& operator *= ( matrix4& a, const matrix4& b )
{
    a = a * b;
    return a;
}

inline float4& operator *= ( float4& a, const matrix4& b )
{
    a = a * b;
    return a;
}







#endif
