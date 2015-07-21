//
//  font_slicer.cpp
//
//  Created by Edmund Kapusniak on 22/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#include "font_slicer.h"
#include <list>
#include <unordered_set>
#include <make_unique.h>
#include <stringf.h>
#include <rect.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_IMAGE_H
#include FT_OUTLINE_H


static const float EPSILON = 0.01f;


//#define DEBUG_SWEEP


/*
    Decompose font shapes into slices.  A slice is a trapezoidal shape
    where the top and bottom edges are horizontal lines, and the left and
    right edges are quadratic bezier curves.
    
    Process:
        Find corner points (where the tangent is discontinuous).
        Find vertical extremes (apex and nadir of curves), splitting outline.
        Perform a plane sweep, visiting each of the points identified in the
            previous steps in y order, to decompose the shape into slices.
        Some slices will have complex curves as the left and right edges.
            Simplify these slices by approximating with quadratic Bezier
            curves, splitting where necessary.
    
    Each slice can be rendered using the GPU - vertical converage is easy, and
    horizontally we can test each pixel against the edges.  We can get a
    fairly accurage approximation of coverage by considering the horizontal
    distance through the centre of the pixel to the slab.  This may give
    more accurate antialiasing at small pixel sizes than distance-based
    methods (since each slice's contribution is considered separately).
    
    This gives us on-GPU rendering of scalable text (though we do not have the
    ability to rotate).  We can render to an FBO to cache glyphs for reuse,
    or just render text directly.

    At least that is the plan.
 
*/



/*
    Path representing a glyph shape which is being processed.
*/



struct path_event;
struct path_vertex;
struct path_edge;
struct path_slice;
struct path;


enum path_event_kind
{
    PATH_END,
    PATH_MOVE_TO,
    PATH_LINE_TO,
    PATH_QUAD_TO,
    PATH_CUBIC_TO,
};


struct path_event
{
    path_event( path_event_kind kind ) : kind( kind ) {}
    path_event( long x, long y ) : p( x, y ) {}

    union
    {
        path_event_kind kind;
        float2 p;
    };
};


struct path_vertex
{
    path_vertex()
    {
        e[ 0 ] = e[ 1 ] = nullptr;
        is_corner = false;
    }

    path_edge* e[ 2 ];
    float2 p;
    bool is_corner;
};


struct path_edge
{
    path_edge()
    {
        kind = PATH_END;
        v[ 0 ] = v[ 1 ] = nullptr;
    }

    path_event_kind kind;
    path_vertex* v[ 2 ];
    float2 c[ 2 ];
};


struct path_slice
{
    path_vertex*    tl;         // vertex at minx, miny
    path_vertex*    tr;         // vertex at maxx, miny
    path_vertex*    bl;         // vertex at minx, maxy
    path_vertex*    br;         // vertex at maxx, maxy
    bool            lreversed;  // left edge is reversed
    bool            rreversed;  // right edge is reversed
    
    qbezier         left;       // approximated left edge
    qbezier         right;      // approximated right edge
};


struct path
{
    path()
    {
        minx = miny = maxx = maxy = -1;
    }


    long minx;
    long miny;
    long maxx;
    long maxy;
    
    std::vector< path_event > p;
    std::vector< path_vertex* > o;
    std::vector< std::unique_ptr< path_vertex > > v;
    std::vector< std::unique_ptr< path_edge > > e;
    std::vector< path_slice > s;
    
};





/*
    Convert FreeType glyph outline to a set of path events.
*/


static int move_to( const FT_Vector* to, void* user )
{
    path* p = (path*)user;
    p->p.emplace_back( PATH_MOVE_TO );
    p->p.emplace_back( to->x, to->y );
    return 0;
}

static int line_to( const FT_Vector* to, void* user )
{
    path* p = (path*)user;
    p->p.emplace_back( PATH_LINE_TO );
    p->p.emplace_back( to->x, to->y );
    return 0;
}

static int conic_to( const FT_Vector* control, const FT_Vector* to, void* user )
{
    path* p = (path*)user;
    p->p.emplace_back( PATH_QUAD_TO );
    p->p.emplace_back( control->x, control->y );
    p->p.emplace_back( to->x, to->y );
    return 0;
}

static int cubic_to( const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user )
{
    path* p = (path*)user;
    p->p.emplace_back( PATH_CUBIC_TO );
    p->p.emplace_back( control1->x, control1->y );
    p->p.emplace_back( control2->x, control2->y );
    p->p.emplace_back( to->x, to->y );
    return 0;
}

static void outline_to_path( path* path, FT_BBox bbox, FT_Outline* outline )
{
    path->minx = bbox.xMin;
    path->miny = bbox.yMin;
    path->maxx = bbox.xMax;
    path->maxy = bbox.yMax;
    
    FT_Outline_Funcs c;
    c.move_to   = move_to;
    c.line_to   = line_to;
    c.conic_to  = conic_to;
    c.cubic_to  = cubic_to;
    c.shift     = 0;
    c.delta     = 0;
    
    FT_Outline_Decompose( outline, &c, path );
    path->p.emplace_back( PATH_END );
}




/*
    Convert set of path events to a connected polygon edge structure.
*/


static void build_polygon( path* path )
{
    // This assumes that the path consists of closed contours with no
    // moving back to a contour that has already been moved away from.

    path_vertex* first = nullptr;
    path_edge* edge = nullptr;
    path_event* last = nullptr;

    for ( size_t i = 0; i < path->p.size(); ++i )
    {
        path_event_kind kind = path->p[ i ].kind;
        
        if ( edge && ( kind == PATH_MOVE_TO || kind == PATH_END ) )
        {
            if ( first && last->p.x == first->p.x && last->p.y == first->p.y )
            {
                // Contour ends at same vertex where it began.
                edge->v[ 1 ] = first;
                first->e[ 0 ] = edge;
            }
            else
            {
                // Contour is open, close it with a line (not ideal...).
                auto v = std::make_unique< path_vertex >();
                auto e = std::make_unique< path_edge >();

                edge->v[ 1 ] = v.get();

                v->e[ 0 ] = edge;
                v->e[ 1 ] = e.get();
                v->p = last->p;
                
                e->kind = PATH_LINE_TO;
                e->v[ 0 ] = v.get();
                e->v[ 1 ] = first;
                
                first->e[ 0 ] = e.get();
                
                path->v.push_back( std::move( v ) );
                path->e.push_back( std::move( e ) );
            }
            
            first = nullptr;
            edge  = nullptr;
            last  = nullptr;
        }


        if ( kind == PATH_MOVE_TO )
        {
            i += 1;
            last = &path->p[ i ];
        }
        else if ( last )
        {
            auto v = std::make_unique< path_vertex >();
            v->e[ 0 ] = edge;
            v->e[ 1 ] = nullptr;
            v->p = last->p;
            
            if ( ! first )
            {
                first = v.get();
                path->o.push_back( v.get() );
            }
            
            if ( edge )
            {
                edge->v[ 1 ] = v.get();
                edge = nullptr;
            }
            
            auto e = std::make_unique< path_edge >();
            e->kind = kind;
            e->v[ 0 ] = v.get();
            v->e[ 1 ] = e.get();

            if ( kind == PATH_LINE_TO )
            {
                i += 1;
                last = &path->p[ i ];
            }
            else if ( kind == PATH_QUAD_TO )
            {
                i += 1;
                e->c[ 0 ] = path->p[ i ].p;

                i += 1;
                last = &path->p[ i ];
            }
            else if ( kind == PATH_CUBIC_TO )
            {
                i += 1;
                e->c[ 0 ] = path->p[ i ].p;

                i += 1;
                e->c[ 1 ] = path->p[ i ].p;

                i += 1;
                last = &path->p[ i ];
            }
            
            edge = e.get();
            
            path->v.push_back( std::move( v ) );
            path->e.push_back( std::move( e ) );
        }
        
    }

}





/*
    Path edge splitting.
*/

static bool split_edge( path* path, path_edge* e, float t )
{
    // Split single edge.

    switch ( e->kind )
    {
    case PATH_LINE_TO:
    {
        lbezier l( e->v[ 0 ]->p, e->v[ 1 ]->p );
        lbezier split[ 2 ];
        l.split( t, split );
        
        float2 s = split[ 0 ].p[ 1 ];
        if ( lengthsq( s - l.p[ 0 ] ) < EPSILON * EPSILON )
            return false;
        if ( lengthsq( s - l.p[ 1 ] ) < EPSILON * EPSILON )
            return false;

        auto v = std::make_unique< path_vertex >();
        auto f = std::make_unique< path_edge >();

        path_vertex* vend = e->v[ 1 ];

        e->c[ 0 ] = split[ 0 ].p[ 1 ];
        e->v[ 1 ] = v.get();
        
        v->e[ 0 ] = e;
        v->e[ 1 ] = f.get();
        v->p = split[ 0 ].p[ 1 ];
        
        f->kind = PATH_LINE_TO;
        f->v[ 0 ] = v.get();
        f->v[ 1 ] = vend;
        
        assert( vend->e[ 0 ] == e );
        vend->e[ 0 ] = f.get();

        path->v.push_back( std::move( v ) );
        path->e.push_back( std::move( f ) );
        return true;
    }
    
    case PATH_QUAD_TO:
    {
        // Split the curve at t, creating a corner vertex.
        qbezier q( e->v[ 0 ]->p, e->c[ 0 ], e->v[ 1 ]->p );
        qbezier split[ 2 ];
        q.split( t, split );
        
        float2 s = split[ 0 ].p[ 2 ];
        if ( lengthsq( s - q.p[ 0 ] ) < EPSILON * EPSILON )
            return false;
        if ( lengthsq( s - q.p[ 2 ] ) < EPSILON * EPSILON )
            return false;
        
        auto v = std::make_unique< path_vertex >();
        auto f = std::make_unique< path_edge >();
        
        path_vertex* vend = e->v[ 1 ];

        e->c[ 0 ] = split[ 0 ].p[ 1 ];
        e->v[ 1 ] = v.get();
        
        v->e[ 0 ] = e;
        v->e[ 1 ] = f.get();
        v->p = split[ 0 ].p[ 2 ];
        
        f->kind = PATH_QUAD_TO;
        f->v[ 0 ] = v.get();
        f->c[ 0 ] = split[ 1 ].p[ 1 ];
        f->v[ 1 ] = vend;
        
        assert( vend->e[ 0 ] == e );
        vend->e[ 0 ] = f.get();
        
        path->v.push_back( std::move( v ) );
        path->e.push_back( std::move( f ) );
        return true;
    }
    
    case PATH_CUBIC_TO:
    {
        cbezier c( e->v[ 0 ]->p, e->c[ 0 ], e->c[ 1 ], e->v[ 1 ]->p );
        cbezier split[ 2 ];
        c.split( t, split );

        float2 s = split[ 0 ].p[ 3 ];
        if ( lengthsq( s - c.p[ 0 ] ) < EPSILON * EPSILON )
            return false;
        if ( lengthsq( s - c.p[ 3 ] ) < EPSILON * EPSILON )
            return false;
        
        auto v = std::make_unique< path_vertex >();
        auto f = std::make_unique< path_edge >();

        path_vertex* vend = e->v[ 1 ];

        e->v[ 1 ] = v.get();
        e->c[ 0 ] = split[ 0 ].p[ 1 ];
        e->c[ 1 ] = split[ 0 ].p[ 2 ];
        
        v->e[ 0 ] = e;
        v->e[ 1 ] = f.get();
        v->p = split[ 0 ].p[ 3 ];
        v->is_corner = true;
        
        f->kind = PATH_CUBIC_TO;
        f->v[ 0 ] = v.get();
        f->c[ 0 ] = split[ 1 ].p[ 1 ];
        f->c[ 1 ] = split[ 1 ].p[ 2 ];
        f->v[ 1 ] = vend;
        
        assert( vend->e[ 0 ] == e );
        vend->e[ 0 ] = f.get();

        path->v.push_back( std::move( v ) );
        path->e.push_back( std::move( f ) );
        return true;
    }
    
    default:
        return false;
    }

}


static path_vertex* split_edge(
        path* path, path_vertex* a, path_vertex* b, bool reversed, float y )
{
    // Split appropriate edge between a and b.  Edge spline must be y-monotone.
    // Y increases from a to b.  reversed is true if we follow edges backwards
    // around the shape, false otherwise.


    // Find edge which intersects horizontal line at a.
    
    path_edge* e = nullptr;
    if ( reversed )
    {
        e = a->e[ 0 ];
        while ( e->v[ 0 ]->p.y < y && e->v[ 0 ] != b )
        {
            e = e->v[ 0 ]->e[ 0 ];
        }
    }
    else
    {
        e = a->e[ 1 ];
        while ( e->v[ 1 ]->p.y < y && e->v[ 1 ] != b )
        {
            e = e->v[ 1 ]->e[ 1 ];
        }
    }


    // Solve at y to find t value at which to split.

    float t = F_NAN;
    switch ( e->kind )
    {
    case PATH_LINE_TO:
    {
        lbezier l( e->v[ 0 ]->p, e->v[ 1 ]->p );
        l.solve_y( y, &t );
        break;
    }
    
    case PATH_QUAD_TO:
    {
        qbezier q( e->v[ 0 ]->p, e->c[ 0 ], e->v[ 1 ]->p );
        float tt[] = { F_NAN, F_NAN };
        q.solve_y( y, tt );
        t = tt[ 0 ];
        break;
    }
    
    case PATH_CUBIC_TO:
    {
        cbezier c( e->v[ 0 ]->p, e->c[ 0 ], e->c[ 1 ], e->v[ 1 ]->p );
        float tt[] = { F_NAN, F_NAN, F_NAN };
        c.solve_y( y, tt );
        t = tt[ 0 ];
        break;
    }
    
    default:
        break;
    }
    
    
    // Attempt to split edge.

    if ( ! isnan( t ) && split_edge( path, e, t ) )
    {
        return e->v[ 1 ];
    }
    
    
    // Otherwise, return the endpoint closest to the split line.
    
    float mid = ( e->v[ 0 ]->p.y + e->v[ 1 ]->p.y ) * 0.5f;
    if ( e->v[ 0 ]->p.y < e->v[ 1 ]->p.y )
    {
        if ( y < mid )
            return e->v[ 0 ];
        else
            return e->v[ 1 ];
    }
    else
    {
        if ( y < mid )
            return e->v[ 1 ];
        else
            return e->v[ 0 ];
    }
    

}


static float solve_edge( path_edge* e, float y )
{
    switch ( e->kind )
    {
    case PATH_LINE_TO:
    {
        lbezier l( e->v[ 0 ]->p, e->v[ 1 ]->p );
        float t = F_NAN;
        size_t i = l.solve_y( y, &t );
        if ( i )
        {
            return l.evaluate( t ).x;
        }
        break;
    }
    
    case PATH_QUAD_TO:
    {
        qbezier q( e->v[ 0 ]->p, e->c[ 0 ], e->v[ 1 ]->p );
        float t[] = { F_NAN, F_NAN };
        size_t i = q.solve_y( y, t );
        if ( i )
        {
            return q.evaluate( t[ 0 ] ).x;
        }
        break;
    }
    
    case PATH_CUBIC_TO:
    {
        cbezier c( e->v[ 0 ]->p, e->c[ 0 ], e->c[ 1 ], e->v[ 1 ]->p );
        float t[] = { F_NAN, F_NAN, F_NAN };
        size_t i = c.solve_y( y, t );
        if ( i )
        {
            return c.evaluate( t[ 0 ] ).x;
        }
        break;
    }
    
    default:
        break;
    }
    
    
    // y should intersect edge, but just in case...
    
    if ( e->v[ 0 ]->p.y < e->v[ 1 ]->p.y )
    {
        if ( y < e->v[ 0 ]->p.y )
            return e->v[ 0 ]->p.x;
        else if ( y > e->v[ 1 ]->p.y )
            return e->v[ 1 ]->p.x;
    }
    else
    {
        if ( y < e->v[ 1 ]->p.y )
            return e->v[ 1 ]->p.x;
        else if ( y > e->v[ 0 ]->p.y )
            return e->v[ 0 ]->p.x;
    }
    
    return ( e->v[ 0 ]->p.x + e->v[ 1 ]->p.x ) * 0.5f;
}





/*
    Split outlines at self-intersections.
*/

static cbezier edge_to_bezier( path_edge* e )
{
    switch ( e->kind )
    {
    case PATH_LINE_TO:
        return lbezier( e->v[ 0 ]->p, e->v[ 1 ]->p );
    case PATH_QUAD_TO:
        return qbezier( e->v[ 0 ]->p, e->c[ 0 ], e->v[ 1 ]->p );
    case PATH_CUBIC_TO:
        return cbezier( e->v[ 0 ]->p, e->c[ 0 ], e->c[ 1 ], e->v[ 1 ]->p );
    default:
        return cbezier();
    }
}

static bool intersect( path* path, path_edge* a, path_edge* b )
{
    cbezier a_bezier = edge_to_bezier( a );
    cbezier b_bezier = edge_to_bezier( b );
    std::pair< float, float > t[ 9 ];
    size_t count = solve_intersection( a_bezier, b_bezier, t );
    
    bool split_a = false;
    bool split_b = false;
    for ( size_t i = 0; i < count; ++i )
    {
        // Ignore cases where curves share a vertex.
        if ( t[ i ].first == 1.0f && t[ i ].second == 0.0f )
            continue;

        // Attempt split at the intersection.
        split_a = split_edge( path, a, t[ i ].first );
        split_b = split_edge( path, b, t[ i ].second );
        if ( split_a || split_b )
            break;
    }
    
    if ( ! split_a && ! split_b )
        return false;
        
        
    /*
                    a
                    v
           b_next   |   b
                 <--+---.
                    |    |
                    \    /
                     '''+
                  a_next
    */
    
    path_edge* a_next;
    path_edge* b_next;
    if ( split_a && split_b )
    {
        // a is before b in the outline.
        a_next = a->v[ 1 ]->e[ 1 ];
        b_next = b->v[ 1 ]->e[ 1 ];
    }
    else if ( split_a )
    {
        a_next = a->v[ 1 ]->e[ 1 ];
        if ( t[ 0 ].second > 0.5f )
        {
            b_next = b->v[ 1 ]->e[ 1 ];
        }
        else
        {
            b_next = b;
            b = b_next->v[ 0 ]->e[ 0 ];
        }
    }
    else if ( split_b )
    {
        b_next = b->v[ 1 ]->e[ 1 ];
        if ( t[ 0 ].first > 0.5f )
        {
            a_next = a->v[ 1 ]->e[ 1 ];
        }
        else
        {
            a_next = a;
            a = a_next->v[ 0 ]->e[ 0 ];
        }
    }

    path_vertex* outer = a->v[ 1 ];
    path_vertex* inner = b->v[ 1 ];

    // Outer loop continues a_prev -> outer -> b_next.
    outer->e[ 1 ] = b_next;
    b_next->v[ 0 ] = outer;
    
    // Inner loop continues b_prev -> inner -> a_next.
    inner->e[ 1 ] = a_next;
    a_next->v[ 0 ] = inner;
    
    // And the inner loop is an independent outline.
    path->o.push_back( inner );
    
    inner->is_corner = true;
    outer->is_corner = true;

    return true;
}

static void self_intersect( path* path, path_vertex* o )
{
restart:
    path_edge* e = o->e[ 1 ];
    while ( true )
    {
        // Intersect edge with all edges before it.
        for ( path_edge* d = o->e[ 1 ]; d != e; d = d->v[ 1 ]->e[ 1 ] )
        {
            if ( intersect( path, d, e ) )
            {
                // Restart after intersection.
                goto restart;
            }
        }
    
        // Move to next edge.
        e = e->v[ 1 ]->e[ 1 ];
        if ( e == o->e[ 1 ] )
        {
            break;
        }
    }
}

static void self_intersect( path* path )
{
    for ( size_t i = 0; i < path->o.size(); ++i )
    {
        self_intersect( path, path->o.at( i ) );
    }
}




/*
    Analyze shape to identify corner points (including splitting y-extremes).
*/

static void find_corners( path* path )
{
    size_t vertex_count = path->v.size();
    size_t edge_count = path->e.size();
    
    for ( size_t i = 0; i < edge_count; ++i )
    {
        path_edge* e = path->e[ i ].get();
        
        // Check for vertical extremes.
        if ( e->kind == PATH_QUAD_TO )
        {
            qbezier q( e->v[ 0 ]->p, e->c[ 0 ], e->v[ 1 ]->p );
            lbezier l = q.derivative();

            float t = F_NAN;
            size_t i = l.solve_y( 0, &t );
            if ( i && split_edge( path, e, t ) )
            {
                e->v[ 1 ]->is_corner = true;
            }

        }
        else if ( e->kind == PATH_CUBIC_TO )
        {
            // First derivative of a cubic bezier curve is a quadratic bezier:
            cbezier c( e->v[ 0 ]->p, e->c[ 0 ], e->c[ 1 ], e->v[ 1 ]->p );
            qbezier h = c.derivative();

            // Maximum or minimum occurs when y component of derivative is 0.
            float t[] = { F_NAN, F_NAN };
            size_t i = h.solve_y( 0, t );

            if ( i >= 1 && split_edge( path, e, t[ 0 ] ) )
            {
                e->v[ 1 ]->is_corner = true;
                e = e->v[ 1 ]->e[ 1 ];
                if ( i >= 2 )
                {
                    // Map t[ 1 ] onto the second split curve (seems ok).
                    t[ 1 ] = ( t[ 1 ] - t[ 0 ] ) / ( 1 - t[ 0 ] );
                }
            }
            
            if ( i >= 2 && split_edge( path, e, t[ 1 ] ) )
            {
                e->v[ 1 ]->is_corner = true;
            }

        }
        
    }


    for ( size_t i = 0; i < vertex_count; ++i )
    {
        path_vertex* v = path->v[ i ].get();
    
/*
        // Make endpoints of any straight line a corner.
        if ( v->e[ 0 ]->kind == PATH_LINE_TO || v->e[ 1 ]->kind == PATH_LINE_TO )
        {
            v->is_corner = true;
            continue;
        }
*/
/*
        // Make any vertex where we transition from a straight line to a
        // curve a corner.
        if (    ( v->e[ 0 ]->kind == PATH_LINE_TO && v->e[ 1 ]->kind != PATH_LINE_TO )
             || ( v->e[ 1 ]->kind == PATH_LINE_TO && v->e[ 0 ]->kind != PATH_LINE_TO ) )
        {
            v->is_corner = true;
            continue;
        }
*/
    
        // Find tangents.
        float2 t0;
        if ( v->e[ 0 ]->kind == PATH_LINE_TO )
        {
            t0 = v->p - v->e[ 0 ]->v[ 0 ]->p;
        }
        else if ( v->e[ 0 ]->kind == PATH_QUAD_TO )
        {
            t0 = v->p - v->e[ 0 ]->c[ 0 ];
        }
        else if ( v->e[ 0 ]->kind == PATH_CUBIC_TO )
        {
            t0 = v->p - v->e[ 0 ]->c[ 1 ];
        }
        
        float2 t1;
        if ( v->e[ 1 ]->kind == PATH_LINE_TO )
        {
            t1 = v->e[ 1 ]->v[ 1 ]->p - v->p;
        }
        else
        {
            t1 = v->e[ 1 ]->c[ 0 ] - v->p;
        }
        
        // Check angle.
        t0 = normalize( t0 );
        t1 = normalize( t1 );
        float cos_theta = dot( t0, t1 );
        static const float LIMIT = cosf( F_TAU * 0.02f );
        if ( cos_theta < LIMIT )
        {
            v->is_corner = true;
        }
        
        // Check for vertical extreme.
        if ( ( t0.y >= 0 && t1.y <= 0 ) || ( t0.y <= 0 && t1.y >= 0 ) )
        {
            v->is_corner = true;
        }
    }
}



/*
    Perform plane sweep to decompose glyph outline into a set of slices.
*/


struct sweep_edge
{
    path_vertex*    top;        // previous corner.
    path_edge*      edge;       // current edge.
    path_vertex*    corner;     // next corner downwards along edge.
    bool            reversed;   // down is either from v[ 0 ] -> v[ 1 ], or reversed
    bool            left;       // edge is either on the left of a filled interval, or on the right
};


static path_vertex* sweep_split(
                path* path, sweep_edge* edge, path_vertex* corner )
{
    // Split the edge at the corner.

    if ( edge->corner == corner )
        return corner;
    
    return split_edge
    (
        path,
        edge->top,
        edge->corner,
        edge->reversed,
        corner->p.y
    );
}


static void sweep_slice(
        path* path, sweep_edge* left, sweep_edge* right, path_vertex* corner )
{
    // Create a slice with the given left and right edges, down to the corner.
    
    assert( left->left );
    assert( ! right->left );
    
    path_slice slice;
    slice.tl          = left->top;
    slice.tr         = right->top;
    slice.bl       = sweep_split( path, left, corner );
    slice.br      = sweep_split( path, right, corner );
    slice.lreversed     = left->reversed;
    slice.rreversed    = right->reversed;
    
    left->top = slice.bl;
    right->top = slice.br;
    
    if ( slice.tl->p.y >= slice.bl->p.y )
    {
        return;
    }

/*
    printf( ">> slice\n" );
    printf( "   %g %g -- %g %g\n",
        slice.tl->p.x, slice.tl->p.y,
        slice.tr->p.x, slice.tr->p.y );
    printf( "   %g %g -- %g %g\n",
        slice.bl->p.x, slice.bl->p.y,
        slice.br->p.x, slice.br->p.y );
*/
    
    path->s.push_back( slice );
}



static void sweep_plane( path* path )
{
    // Sort corners.
    std::vector< path_vertex* > corners;
    for ( size_t i = 0; i < path->v.size(); ++i )
    {
        path_vertex* v = path->v[ i ].get();
        if ( v->is_corner )
        {
            corners.push_back( v );
        }
    }
    
    std::sort
    (
        corners.begin(),
        corners.end(),
        []( path_vertex* a, path_vertex* b )
        {
            return a->p.y < b->p.y || ( a->p.y == b->p.y && a->p.x < b->p.x );
        }
    );
    
    
    // Sweep plane from minimum y to maximum y.  Keep a data structure
    // indicating the intervals which are inside the polygon.
    std::list< sweep_edge > edges;
    for ( size_t i = 0; i < corners.size(); ++i )
    {
        path_vertex* corner = corners[ i ];
        if ( ! corner->is_corner )
            continue;


#ifdef DEBUG_SWEEP
        printf( "corner %p %g %g\n", corner, corner->p.x, corner->p.y );
        for ( auto i = edges.begin(); i != edges.end(); ++i )
        {
            printf( "  %p %g %g : ", i->corner, i->corner->p.x, i->corner->p.y );
            
            printf( "%s ", i->left ? "[" : "]" );
            
            if ( i->reversed )
            {
                printf( "[r] %g %g, ", i->edge->v[ 1 ]->p.x, i->edge->v[ 1 ]->p.y );
            
                switch ( i->edge->kind )
                {
                case PATH_LINE_TO: break;
                case PATH_QUAD_TO:
                    printf( "%g %g, ",
                        i->edge->c[ 0 ].x, i->edge->c[ 0 ].y ); break;
                case PATH_CUBIC_TO:
                    printf( "%g %g, %g %g, ",
                        i->edge->c[ 1 ].x, i->edge->c[ 1 ].y,
                        i->edge->c[ 0 ].x, i->edge->c[ 0 ].y ); break;
                default: break;
                }
                
                printf( "%g %g\n", i->edge->v[ 0 ]->p.x, i->edge->v[ 0 ]->p.y );
            }
            else
            {
                printf( "%g %g, ", i->edge->v[ 0 ]->p.x, i->edge->v[ 0 ]->p.y );
            
                switch ( i->edge->kind )
                {
                case PATH_LINE_TO: break;
                case PATH_QUAD_TO:
                    printf( "%g %g, ",
                        i->edge->c[ 0 ].x, i->edge->c[ 0 ].y ); break;
                case PATH_CUBIC_TO:
                    printf( "%g %g, %g %g, ",
                        i->edge->c[ 0 ].x, i->edge->c[ 0 ].y,
                        i->edge->c[ 1 ].x, i->edge->c[ 1 ].y ); break;
                default: break;
                }
                
                printf( "%g %g\n", i->edge->v[ 1 ]->p.x, i->edge->v[ 1 ]->p.y );
            }
        }
#endif


        // Check if corner is connected to existing edge.
        bool found = false;
        for ( auto i = edges.begin(); i != edges.end(); ++i )
        {
            sweep_edge* e = &*i;
        
            if ( e->corner != corner )
                continue;

            // Check next edge (if it exists).
            auto j = i;
            ++j;
            if ( j != edges.end() && j->corner == corner )
            {
                if ( e->left )
                {
                    /*
                        End of a filled interval:
                        
                            \###/
                             \#/
                              +
                    
                    */

#ifdef DEBUG_SWEEP
                    printf( "END INTERVAL\n" );
#endif

                    sweep_slice( path, e, &*j, corner );
                    
                }
                else
                {
                    /*
                        End of a hole:
                        
                          ##\   /##
                          ###\ /###
                          ####+####
                    */
                    
#ifdef DEBUG_SWEEP
                    printf( "END HOLE\n" );
#endif

                    auto h = i; --h;
                    auto k = j; ++k;

                    sweep_slice( path, &*h, &*i, corner );
                    sweep_slice( path, &*j, &*k, corner );
                
                }
                
                // Remove both edges.
                edges.erase( i );
                edges.erase( j );
                
                found = true;
                break;
            }
            else
            {
                if ( e->left )
                {
                    /*
                            |##
                            +##
                            |##
                    */
                    
#ifdef DEBUG_SWEEP
                    printf( "LEFT\n" );
#endif
                    
                    sweep_slice( path, e, &*j, corner );
                }
                else
                {
                    /*
                          ##|
                          ##+
                          ##|
                    */

#ifdef DEBUG_SWEEP
                    printf( "RIGHT\n" );
#endif
                    
                    auto h = i; --h;
                    sweep_slice( path, &*h, e, corner );
                }
            
                // Move to next corner along edge.
                if ( e->reversed )
                {
                    e->edge = e->corner->e[ 0 ];
                    e->corner = e->edge->v[ 0 ];
                    while ( ! e->corner->is_corner )
                    {
                        e->corner = e->corner->e[ 0 ]->v[ 0 ];
                    }
                }
                else
                {
                    e->edge = e->corner->e[ 1 ];
                    e->corner = e->edge->v[ 1 ];
                    while ( ! e->corner->is_corner )
                    {
                        e->corner = e->corner->e[ 1 ]->v[ 1 ];
                    }
                }
                
                found = true;
                break;
            }

        }
        
        if ( found )
            continue;
        
        // Find the interval containing the corner.
        auto after = edges.begin();
        for ( ; after != edges.end(); ++after )
        {
            sweep_edge* e = &*after;
        
            // Follow edge until we found one that intersects the plane.
            path_edge* original = e->edge;
            if ( e->reversed )
            {
                while ( e->edge->v[ 0 ]->p.y < corner->p.y )
                {
                    e->edge = e->edge->v[ 0 ]->e[ 0 ];
                    if ( e->edge == original )
                    {
                        // Bad case.
                        break;
                    }
                }
            }
            else
            {
                while ( e->edge->v[ 1 ]->p.y < corner->p.y )
                {
                    e->edge = e->edge->v[ 1 ]->e[ 1 ];
                    if ( e->edge == original )
                    {
                        // Bad case.
                        break;
                    }
                }
            }


            // Work out where plane intersects this edge.
            float x = solve_edge( e->edge, corner->p.y );
            
            
            // If the corner is left of the edge, then this edge is after the
            // corner.
            if ( corner->p.x < x )
            {
                break;
            }
            
        }

        bool is_hole = false;
        if ( after != edges.end() && ! after->left )
        {
            /*
                Start of a hole.
                
                  ####+####
                  ###/ \###
                  ##/   \##
            */
            
#ifdef DEBUG_SWEEP
            printf( "START HOLE\n" );
#endif

            is_hole = true;
        }
        else
        {
            /*
                Start of a new filled interval.
                
                      +
                     /#\
                    /###\
            */
            
#ifdef DEBUG_SWEEP
            printf( "START INTERVAL\n" );
#endif
            
            is_hole = false;
        }
        
        
        // corner->e[ 0 ] and corner->[ 1 ] are new edges.
        sweep_edge left;
        left.top = corner;
        left.edge = corner->e[ 0 ];
        left.corner = left.edge->v[ 0 ];
        left.reversed = true;
        left.left = false;
        
        while ( ! left.corner->is_corner )
        {
            left.corner = left.corner->e[ 0 ]->v[ 0 ];
        }
        
        sweep_edge right;
        right.top = corner;
        right.edge = corner->e[ 1 ];
        right.corner = right.edge->v[ 1 ];
        right.reversed = false;
        right.left = false;

        while ( ! right.corner->is_corner )
        {
            right.corner = right.corner->e[ 1 ]->v[ 1 ];
        }


        // Work out which edge is to the left.  If we get this wrong then
        // the topology of the figure breaks and we get corrupted slices or
        // encounter the bad case above.
        
        // Look at control point closest to corner.
        float2 tleft;
        switch ( corner->e[ 0 ]->kind )
        {
        default:            tleft = corner->e[ 0 ]->v[ 0 ]->p;      break;
        case PATH_QUAD_TO:  tleft = corner->e[ 0 ]->c[ 0 ];         break;
        case PATH_CUBIC_TO: tleft = corner->e[ 0 ]->c[ 1 ];         break;
        }
        
        float2 tright;
        switch ( corner->e[ 1 ]->kind )
        {
        default:            tright = corner->e[ 1 ]->v[ 1 ]->p;     break;
        case PATH_QUAD_TO:  tright = corner->e[ 1 ]->c[ 0 ];        break;
        case PATH_CUBIC_TO: tright = corner->e[ 1 ]->c[ 0 ];        break;
        }

        tleft = normalize( tleft - corner->p );
        tright = normalize( tright - corner->p );

        if ( tleft.x > tright.x )
        {
            std::swap( left, right );
        }

        
        if ( is_hole )
            right.left = true;
        else
            left.left = true;


        if ( is_hole )
        {
            // Check for non-zero winding.  It only actually creates a hole
            // if the winding is opposite to the winding of the figure it's
            // contained in.
            auto before = after; --before;
        
            assert( before->reversed != after->reversed );
            assert( left.reversed != right.reversed );
            
            if ( before->reversed == left.reversed )
            {
                // Snip this entire loop from the figure, as it may well have
                // been caused by a self-intersection.
                path_vertex* v = corner;
                do
                {
//                    printf( "  NO CORNER: %p %g %g\n", v, v->p.x, v->p.y );
                    v->is_corner = false;
                    v = v->e[ 1 ]->v[ 1 ];
                }
                while ( v != corner );
                continue;
            }
        
            // End slice above the hole.
            sweep_slice( path, &*before, &*after, corner );
        }
        
        edges.insert( after, left );
        edges.insert( after, right );
        
    }
        

}



/*
    Approximate full slices with slices with quadratic bezier edges.
*/


static float approx_solve(
        path_vertex* a, path_vertex* b, bool reversed, float y )
{
    if ( y <= a->p.y )
        return a->p.x;
    if ( y >= b->p.y )
        return b->p.x;

    path_edge* e = nullptr;
    if ( reversed )
    {
        e = a->e[ 0 ];
        while ( e->v[ 0 ] != b && e->v[ 0 ]->p.y < y )
        {
            e = e->v[ 0 ]->e[ 0 ];
        }
    }
    else
    {
        e = a->e[ 1 ];
        while ( e->v[ 1 ] != b && e->v[ 1 ]->p.y < y )
        {
            e = e->v[ 1 ]->e[ 1 ];
        }
    }
    
    return solve_edge( e, y );
}


static float approx_error(
        path_vertex* a, path_vertex* b, bool reversed, qbezier* approx )
{
    /*
        Approximate error by accumulating the horizontal distance to the real
        line at various values of t.
    */

    static const float SAMPLES = 16;
    float error = 0;
    for ( float t = 1.0f / SAMPLES; t < 1.0f; t += 1.0f / SAMPLES )
    {
        float2 p = approx->evaluate( t );
        float x = approx_solve( a, b, reversed, p.y );
        error += fabsf( p.x - x );
    }
    
    return error / SAMPLES;
}


static bool approx_slice(
        path_vertex* a, path_vertex* b, bool reversed, qbezier* out )
{
    float2 ta;
    float2 tb;
    
    if ( reversed )
    {
        switch ( a->e[ 0 ]->kind )
        {
        case PATH_LINE_TO:  ta = a->e[ 0 ]->v[ 0 ]->p;  break;
        case PATH_QUAD_TO:  ta = a->e[ 0 ]->c[ 0 ];     break;
        case PATH_CUBIC_TO: ta = a->e[ 0 ]->c[ 1 ];     break;
        default: break;
        }
        
        switch ( b->e[ 1 ]->kind )
        {
        case PATH_LINE_TO:  tb = b->e[ 1 ]->v[ 1 ]->p;  break;
        case PATH_QUAD_TO:  tb = b->e[ 1 ]->c[ 0 ];     break;
        case PATH_CUBIC_TO: tb = b->e[ 1 ]->c[ 0 ];     break;
        default: break;
        }
    }
    else
    {
        switch ( a->e[ 1 ]->kind )
        {
        case PATH_LINE_TO:  ta = a->e[ 1 ]->v[ 1 ]->p;  break;
        case PATH_QUAD_TO:  ta = a->e[ 1 ]->c[ 0 ];     break;
        case PATH_CUBIC_TO: ta = a->e[ 1 ]->c[ 0 ];     break;
        default: break;
        }
    
        switch ( b->e[ 0 ]->kind )
        {
        case PATH_LINE_TO:  tb = b->e[ 0 ]->v[ 0 ]->p;  break;
        case PATH_QUAD_TO:  tb = b->e[ 0 ]->c[ 0 ];     break;
        case PATH_CUBIC_TO: tb = b->e[ 0 ]->c[ 1 ];     break;
        default: break;
        }
    }
    
    ta = normalize( ta - a->p );
    tb = normalize( tb - b->p );
    
    float2 c = b->p - a->p;
  
    /*
        Intersection of two lines:
        
        http://math.stackexchange.com/questions/406864/intersection-of-two-lines-in-vector-form
        http://en.wikipedia.org/wiki/Cramer's_rule
    
        | ta.x -tb.x | | s | = | c.x |
        | ta.y -tb.y | | t |   | c.y |
     
    */
    
    float sdet = c.x * -tb.y - -tb.x * c.y;
    float tdet = ta.x * c.y - c.x * ta.y;
    float det = ta.x * -tb.y - -tb.x * ta.y;
    
    if ( ( fabsf( sdet ) < EPSILON && fabsf( tdet ) < EPSILON )
            || fabsf( det ) < EPSILON )
    {
        // Probably linear.    
        *out = qbezier
        (
            a->p,
            ( a->p + b->p ) * 0.5f,
            b->p
        );
        return true;
    }
        
    if ( fabsf( det ) > EPSILON )
    {
        float s = sdet / det;
        float t = tdet / det;
        
        if ( s > EPSILON && t > EPSILON )
        {
            *out = qbezier
            (
                a->p,
                a->p + s * ta,
                b->p
            );
            return true;
        }
    }


    // Tangents don't meet at a point (or, meeting point is behind).  Return
    // a line.
    
    *out = qbezier
    (
        a->p,
        ( a->p + b->p ) * 0.5f,
        b->p
    );

    return false;
}


static void approx_split( path* path, path_slice* s )
{
    static const float MINSPLIT = 10.0f;
    static const float MAXERROR = 2.5f;


    // Attempt approximation.
    bool lvalid = approx_slice( s->tl, s->bl, s->lreversed, &s->left );
    bool rvalid = approx_slice( s->tr, s->br, s->rreversed, &s->right );
    

    // Errors in approximation can cause sides of slice to have different extents.
    s->right.p[ 0 ].y = s->left.p[ 0 ].y;
    s->right.p[ 2 ].y = s->left.p[ 2 ].y;
    
    
    // Check approximation.
    if ( lvalid && approx_error( s->tl, s->bl, s->lreversed, &s->left ) < MAXERROR
         && rvalid && approx_error( s->tr, s->br, s->rreversed, &s->right ) < MAXERROR )
    {
        return;
    }


    // Don't split forever.
    if ( s->bl->p.y - s->tl->p.y <= MINSPLIT )
    {
        return;
    }

    
    // Otherwise, split.
    float split_y = ( s->tl->p.y + s->bl->p.y ) * 0.5f;
    path_vertex* lv = split_edge( path, s->tl, s->bl, s->lreversed, split_y );
    path_vertex* rv = split_edge( path, s->tr, s->br, s->rreversed, split_y );

    
    // If the split failed (picked one of the corner vertices), return.
    if ( lv == s->tl || lv == s->bl || rv == s->tr || rv == s->br )
    {
        return;
    }
    

    // Continue approximating both split slices.
    path_slice slice;
    slice.tl = lv;
    slice.tr = rv;
    slice.bl = s->bl;
    slice.br = s->br;
    slice.lreversed = s->lreversed;
    slice.rreversed = s->rreversed;
    
    s->bl = lv;
    s->br = rv;
    
    approx_split( path, s );
    approx_split( path, &slice );
    
    path->s.push_back( slice );
    

}



static void approx( path* path )
{
    size_t slice_count = path->s.size();
    for ( size_t i = 0; i < slice_count; ++i )
    {
        path_slice* s = &path->s[ i ];
        approx_split( path, s );
    }
}




/*
    Write out a path as SVG.
*/

static void write_svg( path* path, const char* filename )
{
    FILE* f = fopen( filename, "w" );
    fprintf( f, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n" );
    
    fprintf( f, "  <path stroke=\"black\" stroke-width=\"5\" fill=\"none\" d=\"\n" );
    for ( size_t i = 0; i < path->p.size(); )
    {
        const path_event& e = path->p.at( i++ );
        switch ( e.kind )
        {
        case PATH_END:
            break;
            
        case PATH_MOVE_TO:
        {
            float2 p = path->p.at( i++ ).p;
            fprintf( f, "    M %g %g\n", p.x, p.y );
            break;
        }
        
        case PATH_LINE_TO:
        {
            float2 p = path->p.at( i++ ).p;
            fprintf( f, "    L %g %g\n", p.x, p.y );
            break;
        }
        
        case PATH_QUAD_TO:
        {
            float2 a = path->p.at( i++ ).p;
            float2 p = path->p.at( i++ ).p;
            fprintf( f, "    Q %g %g %g %g\n", a.x, a.y, p.x, p.y );
            break;
        }
        
        case PATH_CUBIC_TO:
        {
            float2 a = path->p.at( i++ ).p;
            float2 b = path->p.at( i++ ).p;
            float2 p = path->p.at( i++ ).p;
            fprintf( f, "    C %g %g %g %g %g %g\n", a.x, a.y, b.x, b.y, p.x, p.y );
            break;
        }
        
        }
    }
    fprintf( f, "  />\n" );
    
    for ( const auto& vertex : path->v )
    {
        if ( ! vertex->is_corner )
            continue;
        fprintf( f, "  <circle cx=\"%g\" cy=\"%g\" r=\"20\" "
            "stroke=\"red\" stroke-width=\"5\" fill=\"none\" />\n",
                vertex->p.x, vertex->p.y );
    }
    
    for ( const auto& slice : path->s )
    {
        fprintf( f, "  <path stroke=\"blue\" stroke-width=\"5\" fill=\"none\" "
            "d=\"M %g %g L %g %g L %g %g L %g %g Z\" />\n",
                slice.tl->p.x, slice.tl->p.y,
                slice.tr->p.x, slice.tr->p.y,
                slice.br->p.x, slice.br->p.y,
                slice.bl->p.x, slice.bl->p.y );
                
    }
    
    fprintf( f, "</svg>\n" );
    fclose( f );
}








/*
    font-slicer class.
*/


struct font_slicer::impl
{
    impl()
        :   library( nullptr )
        ,   face( nullptr )
    {
    }


    FT_Library  library;
    FT_Face     face;

    std::vector< char32_t >  glyphs;
    std::vector< font_kern > kerning;
};


font_slicer::font_slicer( const char* path )
    :   p( new impl() )
{
    // Open FreeType library and load font.
    FT_Init_FreeType( &p->library );
    FT_New_Face( p->library, path, 0, &p->face );

    // Get list of all glyphs in font.
    FT_UInt glyph_index = 0;
    FT_ULong char_code = FT_Get_First_Char( p->face, &glyph_index );
    while ( glyph_index )
    {
        p->glyphs.push_back( (char32_t)char_code );
        char_code = FT_Get_Next_Char( p->face, char_code, &glyph_index );
    }
    
    // Parse all kerning information.
    for ( size_t i = 0; i < p->glyphs.size(); ++i )
    {
        char32_t a = p->glyphs[ i ];
        for ( size_t j = 0; j < p->glyphs.size(); ++j )
        {
            char32_t b = p->glyphs[ j ];
            FT_Vector k;
            FT_Get_Kerning( p->face, a, b, FT_KERNING_UNSCALED, &k );
            if ( k.x )
            {
                font_kern kern;
                kern.a = a;
                kern.b = b;
                kern.kerning = k.x;
                p->kerning.push_back( kern );
            }
        }
    }
    
}

font_slicer::~font_slicer()
{
    FT_Done_Face( p->face );
    FT_Done_FreeType( p->library );
}

    
float font_slicer::units_per_em()
{
    return p->face->units_per_EM;
}

float font_slicer::ascender()
{
    return p->face->ascender;
}

float font_slicer::descender()
{
    return p->face->descender;
}

float font_slicer::line_height()
{
    return p->face->height;
}


size_t font_slicer::glyph_count()
{
    return p->glyphs.size();
}

char32_t font_slicer::glyph_char( size_t index )
{
    return p->glyphs.at( index );
}

font_glyph font_slicer::glyph_info( size_t index )
{
    return glyph_info_for_char( p->glyphs.at( index ) );
}

font_glyph font_slicer::glyph_info_for_char( char32_t c )
{
//    printf( "***** %c\n", (char)c );

    // Load character image.
    FT_Load_Char( p->face, c, FT_LOAD_NO_SCALE );
    
    // Make it bolder.
//    FT_Outline_Embolden( &p->face->glyph->outline, 5 * ( 1 << 6 ) / 2 );

    // Process path.
    path path;
    outline_to_path( &path, p->face->bbox, &p->face->glyph->outline );
    build_polygon( &path );
    self_intersect( &path );
    find_corners( &path );
    sweep_plane( &path );

    // Debug.
//    write_svg( &path, stringf( "c%02X.svg", (int)c ).c_str() );
    

    approx( &path );
    
    // Return sliced glyph.
    font_glyph g;
    g.c = c;
    g.advance = p->face->glyph->advance.x;
    g.bounds.minx = path.minx;
    g.bounds.miny = path.miny;
    g.bounds.maxx = path.maxx;
    g.bounds.maxy = path.maxy;
    for ( size_t i = 0; i < path.s.size(); ++i )
    {
        font_slice slice;
        slice.left = path.s[ i ].left;
        slice.right = path.s[ i ].right;
        g.slices.push_back( slice );
    }
    
    std::sort
    (
        g.slices.begin(),
        g.slices.end(),
        [] ( const font_slice& a, const font_slice& b )
        {
            return a.left.p[ 0 ].y < b.left.p[ 0 ].y;
        }
    );
    
    return g;
}
    

size_t font_slicer::kern_count()
{
    return p->kerning.size();
}

font_kern font_slicer::kern( size_t index )
{
    return p->kerning.at( index );
}



