//
// region.h
//
// Created 17th September 2013.
// (c) Edmund Kapusniak 2013.  All rights reserved.
//



#ifndef REGION_H
#define REGION_H



#include <new>
#include <unordered_set>
#include <intmath.h>


#ifdef __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #define __thread
    #else
        #define __thread __thread
    #endif
#else
#define __thread __thread
#endif


class region;
class region_scope;
class region_buffer;
template < class T > struct region_allocator;



extern __thread region* region_current;



class region
{
public:

    static const size_t BLOCK_SIZE  = 4 * 1024 * 1024;
    static const size_t ALIGNMENT   = 8;

    region();
    ~region();

    size_t      maxalloc();
    void*       malloc( size_t new_size );
    void*       realloc( void* p, size_t old_size, size_t new_size );
    const char* strdup( const char* string );
    void        free( void* p, size_t old_size );


private:

    region( const region& );
    region& operator = ( const region& );

    void*   _malloc( size_t new_size );
    void*   _realloc( void* p, size_t old_size, size_t new_size );
    void    _free( void* p, size_t old_size );

    std::unordered_set< void* > allocs;
    char*   block;
    size_t  next;

};


class region_buffer
{
public:

    explicit region_buffer( region& region );
    ~region_buffer();

    void    clear();
    void    append( char c );
    void    append( uint8_t c );
    void    append( const void* data, size_t size );
    size_t  size();
    void*   get();
    
    void    shrink();
    void*   tearoff();

private:

    void    _expand( size_t new_capacity );

    region* region;
    char*   buffer;
    size_t  capacity;
    size_t  index;

};



void* operator new ( size_t size, region& region );
void  operator delete ( void* p, region& region );







class region_scope
{
public:

    region_scope( region& region );
    ~region_scope();

private:

    region* previous;

};



template < typename T >
class region_allocator
{
public:

    typedef T           value_type;
    typedef ptrdiff_t   difference_type;
    typedef size_t      size_type;

    T*      allocate( size_t n );
    void    deallocate( T* p, size_t n );

};






inline size_t region::maxalloc()
{
    return BLOCK_SIZE - next;
}

inline void* region::malloc( size_t new_size )
{
    new_size = align( new_size, ALIGNMENT );

    if ( next + new_size <= BLOCK_SIZE )
    {
        void* p = block + next;
        next += new_size;
        return p;
    }
    else
    {
        return _malloc( new_size );
    }
}

inline void* region::realloc( void* p, size_t old_size, size_t new_size )
{
    old_size = align( old_size, ALIGNMENT );
    new_size = align( new_size, ALIGNMENT );

    if ( p == NULL )
        return malloc( new_size );

    if ( new_size == 0 )
        return free( p, old_size ), (void*)NULL;

    if ( old_size < next
            && (char*)p == block + next - old_size
            && next - old_size + new_size <= BLOCK_SIZE )
    {
        next -= old_size;
        next += new_size;
        return p;
    }
    else
    {
        return _realloc( p, old_size, new_size );
    }
}

inline const char* region::strdup( const char* string )
{
    size_t length = strlen( string );
    char* s = (char*)malloc( length + 1 );
    memcpy( s, string, length );
    s[ length ] = '\0';
    return s;
}

inline void region::free( void* p, size_t old_size )
{
    if ( p == NULL )
        return;

    old_size = align( old_size, ALIGNMENT );

    if ( (char*)p + old_size == block + next )
    {
        next = next - old_size;
    }
    else
    {
        _free( p, old_size );
    }
}





inline void region_buffer::clear()
{
    index = 0;
}

inline void region_buffer::append( char c )
{
    if ( index >= capacity )
    {
        _expand( index + 1 );
    }

    buffer[ index++ ] = c;
}

inline void region_buffer::append( uint8_t c )
{
    if ( index >= capacity )
    {
        _expand( index + 1 );
    }

    buffer[ index++ ] = c;
}

inline void region_buffer::append( const void* data, size_t size )
{
    if ( index + size > capacity )
    {
        _expand( index + size );
    }
    
    memcpy( buffer + index, data, size );
    index += size;
}


inline size_t region_buffer::size()
{
    return index;
}

inline void* region_buffer::get()
{
    return buffer;
}


inline void* operator new ( size_t size, region& region )
{
    return region.malloc( size );
}

inline void operator delete ( void* p, region& region )
{
    // don't know the size so can't free.
}





inline region_scope::region_scope( region& region )
    :   previous( region_current )
{
    region_current = &region;
}

inline region_scope::~region_scope()
{
    region_current = previous;
}



template < typename T >
inline T* region_allocator< T >::allocate( size_t n )
{
    return (T*)region_current->malloc( sizeof( T ) * n );
}

template < typename T >
inline void region_allocator< T >::deallocate( T* p, size_t n )
{
    region_current->free( p, sizeof( T ) * n );
}





#endif

