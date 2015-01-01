//
//  hashtable.h
//
//  Created by Edmund Kapusniak on 26/10/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef HASHTABLE_H
#define HASHTABLE_H


#include <algorithm>
#include <iterator>
#include <limits>
#include <new>
#include "intmath.h"


/*
    This is a hash table which uses open addressing while ensuring that
    spilled elements do not displace non-colliding elements.  See the design
    of Lua's tables and the following paper:

        http://www.lua.org/source/5.2/ltable.c.html
        http://maths-people.anu.edu.au/~brent/pd/rpb013.pdf

    The interface isn't compatible with std::unordered_map.
*/


template < typename key_t, typename value_t >
class hashtable
{
public:

    struct keyval
    {
        keyval( const key_t& key, const value_t& value );
    
        const key_t key;
        value_t     value;
    };
    

private:

    struct bucket
    {
        static bucket* const EMPTY;
    
        alignas( keyval ) char data[ sizeof( keyval ) ];
        bucket* next;
    };


    template < typename result_t >
    class basic_iterator
        :   public std::iterator< std::forward_iterator_tag, result_t >
    {
    public:
    
        basic_iterator( const basic_iterator< const keyval >& iterator );
        
        bool operator != ( const basic_iterator& b );
        
        basic_iterator& operator ++ ();
        basic_iterator operator ++ ( int );
        
        result_t& operator * () const;
        result_t* operator -> () const;
        
    private:
    
        friend class hashtable;
        
        explicit basic_iterator( bucket* current, bucket* end );
    
        bucket* current;
        bucket* end;

    };
    
    
public:

    typedef key_t key_type;
    typedef value_t mapped_type;
    typedef keyval value_type;
    typedef keyval& reference;
    typedef const keyval& const_reference;
    typedef basic_iterator< keyval > iterator;
    typedef basic_iterator< const keyval > const_iterator;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;


    hashtable();
    hashtable( hashtable&& h );
    hashtable( const hashtable& h );
    hashtable& operator = ( hashtable&& h );
    hashtable& operator = ( const hashtable& h );
    ~hashtable();
    
    size_type       size() const;
    size_type       max_size() const;
    bool            empty() const;
    
    const_iterator  cbegin() const;
    const_iterator  begin() const;
    const_iterator  cend() const;
    const_iterator  end() const;

    const keyval*   lookup( const key_t& key ) const;

    iterator        begin();
    iterator        end();

    keyval*         lookup( const key_t& key );

    void            insert( const key_t& key, const value_t& value );
    void            remove( const key_t& key );
    void            clear();
    
    void            swap( hashtable& h );


private:

    void            rehash( size_t new_capacity );
    bucket*         main_position( const key_t& key );
    bucket*         free_position( bucket* near );

    size_t          capacity;
    size_t          occupancy;
    bucket*         buckets;
    
};


template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::bucket* const
            hashtable< key_t, value_t >::bucket::EMPTY =
                        (typename hashtable< key_t, value_t >::bucket*)-1;


template < typename key_t, typename value_t >
template < typename result_t >
hashtable< key_t, value_t >::basic_iterator< result_t >::basic_iterator(
                bucket* current, bucket* end )
    :   current( current )
    ,   end( end )
{
    while ( current != end && current->next == bucket::EMPTY )
    {
        current += 1;
    }
}

template < typename key_t, typename value_t >
template < typename result_t >
hashtable< key_t, value_t >::basic_iterator< result_t >::basic_iterator(
                const basic_iterator< const keyval >& iterator )
    :   current( iterator.current )
    ,   end( iterator.end )
{
}

template < typename key_t, typename value_t >
template < typename result_t >
bool hashtable< key_t, value_t >::basic_iterator< result_t >::operator != (
                const basic_iterator& b )
{
    return current != b.current;
}

template < typename key_t, typename value_t >
template < typename result_t >
hashtable< key_t, value_t >::basic_iterator< result_t >&
        hashtable< key_t, value_t >::basic_iterator< result_t >::operator ++ ()
{
    while ( current != end )
    {
        current += 1;
        if ( current->next != bucket::EMPTY )
            break;
    }
    return *this;
}

template < typename key_t, typename value_t >
template < typename result_t >
hashtable< key_t, value_t >::basic_iterator< result_t >
    hashtable< key_t, value_t >::basic_iterator< result_t >::operator ++ ( int )
{
    basic_iterator result( *this );
    operator ++ ();
    return result;
}

template < typename key_t, typename value_t >
template < typename result_t >
result_t& hashtable< key_t, value_t >::basic_iterator< result_t >::operator * () const
{
    assert( current->next != bucket::EMPTY );
    return *(keyval*)current->data;
}

template < typename key_t, typename value_t >
template < typename result_t >
result_t* hashtable< key_t, value_t >::basic_iterator< result_t >::operator -> () const
{
    assert( current->next != bucket::EMPTY );
    return (keyval*)current->data;
}


template < typename key_t, typename value_t >
hashtable< key_t, value_t >::hashtable()
    :   capacity( 0 )
    ,   occupancy( 0 )
    ,   buckets( NULL )
{
}

template < typename key_t, typename value_t >
hashtable< key_t, value_t >::hashtable( hashtable&& h ) : hashtable()
{
    swap( h );
}

template < typename key_t, typename value_t >
hashtable< key_t, value_t >::hashtable( const hashtable& h ) : hashtable()
{
    operator = ( h );
}

template < typename key_t, typename value_t >
hashtable< key_t, value_t >&
                hashtable< key_t, value_t >::operator = ( hashtable&& h )
{
    swap( h );
    return *this;
}

template < typename key_t, typename value_t >
hashtable< key_t, value_t >&
                hashtable< key_t, value_t >::operator = ( const hashtable& h )
{
    clear();
    for ( auto i = h.begin(); i != h.end(); ++i )
    {
        insert( i->key, i->value );
    }
    return *this;
}

template < typename key_t, typename value_t >
hashtable< key_t, value_t >::~hashtable()
{
    clear();
    free( buckets );
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::size_type
                hashtable< key_t, value_t >::size() const
{
    return occupancy;
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::size_type
                hashtable< key_t, value_t >::max_size() const
{
    return std::numeric_limits< size_type >::max();
}

template < typename key_t, typename value_t >
bool hashtable< key_t, value_t >::empty() const
{
    return occupancy == 0;
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::const_iterator
                hashtable< key_t, value_t >::cbegin() const
{
    return const_iterator( buckets, buckets + occupancy );
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::const_iterator
                hashtable< key_t, value_t >::begin() const
{
    return cbegin();
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::const_iterator
                hashtable< key_t, value_t >::cend() const
{
    return const_iterator( buckets + occupancy, buckets + occupancy );
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::const_iterator
                hashtable< key_t, value_t >::end() const
{
    return cend();
}

template < typename key_t, typename value_t >
const typename hashtable< key_t, value_t >::keyval*
                hashtable< key_t, value_t >::lookup( const key_t& key ) const
{
    bucket* lookup = main_position( key );
    if ( lookup->next == bucket::EMPTY )
        return nullptr;
    while ( lookup )
    {
        keyval* kv = (keyval*)lookup->data;
        if ( kv->key == key )
            return kv;
        lookup = lookup->next;
    }
    return nullptr;
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::iterator
                hashtable< key_t, value_t >::begin()
{
    return iterator( buckets, buckets + occupancy );
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::iterator
                hashtable< key_t, value_t >::end()
{
    return iterator( buckets + occupancy, buckets + occupancy );
}

template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::keyval*
                hashtable< key_t, value_t >::lookup( const key_t& key )
{
    if ( occupancy == 0 )
        return nullptr;
    bucket* lookup = main_position( key );
    if ( lookup->next == bucket::EMPTY )
        return nullptr;
    while ( lookup )
    {
        keyval* kv = (keyval*)lookup->data;
        if ( kv->key == key )
            return kv;
        lookup = lookup->next;
    }
    return nullptr;
}

template < typename key_t, typename value_t >
void hashtable< key_t, value_t >::insert( const key_t& key, const value_t& value )
{
    // Lookup in case key already exists.
    keyval* lk = lookup( key );
    if ( lk )
    {
        lk->value = value;
        return;
    }

    // Grow hash if necessary.  Possibly show grow at lower occupancy rather
    // than waiting until the hash is completely full.
    if ( occupancy >= capacity )
    {
        rehash( occupancy + 1 );
    }

    // Occupancy will increase.
    occupancy += 1;

    // Check if element can be placed in its main position.
    bucket* insert = main_position( key );
    if ( insert->next == bucket::EMPTY )
    {
        // Bucket is no longer empty.
        insert->next = nullptr;
    }
    else
    {
        // Find main position of occupying element.
        keyval* kv = (keyval*)insert->data;
        bucket* cuckoo = main_position( kv->key );
        assert( cuckoo->next != bucket::EMPTY );
        
        // Find free bucket near the main position of the occupying element.
        bucket* free = free_position( cuckoo );
        
        if ( cuckoo == insert )
        {
            // Real collision, place new element in free position and link it
            // into the list starting from insert.
            free->next = insert->next;
            insert->next = free;
            insert = free;
        }
        else
        {
            // cuckoo and insert have the same hash.  cuckoo is the bucket
            // at the head of the linked list for this hash.  We want to move
            // the occupying element (in insert) to the free bucket.
            
            // Replace insert with free in cuckoo's linked list.
            bucket* previous = cuckoo;
            while ( previous->next != insert )
            {
                previous = previous->next;
            }
            previous->next = free;
            free->next = insert->next;
        
            // Move occupying element from insert to free.
            new ( free->data ) keyval( std::move( *kv ) );
            kv->~keyval();
            
            // insert bucket is the head of an empty list.
            insert->next = nullptr;
        }
    }
    
    // insert is now the correct location in which to place the element.
    new ( insert->data ) keyval( key, value );
}

template < typename key_t, typename value_t >
void hashtable< key_t, value_t >::remove( const key_t& key )
{
    if ( ! occupancy )
        return;

    bucket* lookup = main_position( key );

    if ( lookup->next == bucket::EMPTY )
    {
        return;
    }
    
    keyval* kv = (keyval*)lookup->data;
    if ( kv->key == key )
    {
        kv->~keyval();
        if ( lookup->next )
        {
            // Move next item in list into head position.
            keyval* kv = (keyval*)lookup->next->data;
            new ( lookup->data ) keyval( std::move( *kv ) );
            kv->~keyval();
            
            // And fix the linked list.
            bucket* next = lookup->next->next;
            lookup->next->next = bucket::EMPTY;
            lookup->next = next;
        }
        else
        {
            // No items in table with this hash.
            lookup->next = bucket::EMPTY;
        }
        return;
    }
    
    while ( lookup->next )
    {
        bucket* previous = lookup;
        lookup = lookup->next;

        keyval* kv = (keyval*)lookup->data;
        if ( kv->key == key )
        {
            // Delete this item.
            kv->~keyval();
            
            // And skip over it in the linked list.
            previous->next = lookup->next;
            lookup->next = bucket::EMPTY;
            return;
        }
    }
}

template < typename key_t, typename value_t >
void hashtable< key_t, value_t >::clear()
{
    if ( ! std::is_trivially_destructible< keyval >::value )
    {
        for ( size_t i = 0; i < capacity; ++i )
        {
            bucket* bucket = buckets + i;
            if ( bucket->next != bucket::EMPTY )
            {
                ( (keyval*)bucket->data )->~keyval();
            }
        }
    }
    
    occupancy = 0;
}

template < typename key_t, typename value_t >
void hashtable< key_t, value_t >::swap( hashtable& h )
{
    std::swap( capacity, h.capacity );
    std::swap( occupancy, h.occupancy );
    std::swap( buckets, h.buckets );
}


template < typename key_t, typename value_t >
void hashtable< key_t, value_t >::rehash( size_t new_capacity )
{
    // Remember old hash.
    size_t old_capacity = capacity;
    bucket* old_buckets = buckets;
    
    // Allocate new hash.
    capacity = std::max( new_capacity, occupancy );
    capacity = ceil_pow2( capacity );
    occupancy = 0;
    buckets = (bucket*)malloc( capacity * sizeof( bucket ) );
    for ( size_t i = 0; i < capacity; ++i )
    {
        buckets[ i ].next = bucket::EMPTY;
    }
    
    // Reinsert elements.
    for ( size_t i = 0; i < old_capacity; ++i )
    {
        bucket* bucket = old_buckets + i;
        if ( bucket->next != bucket::EMPTY )
        {
            keyval* kv = (keyval*)bucket->data;
            insert( kv->key, kv->value );
            kv->~keyval();
        }
    }
    
    // Delete old elements.
    free( old_buckets );
}


template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::bucket*
                hashtable< key_t, value_t >::main_position( const key_t& key )
{
    std::hash< key_t > hasher;
    size_t hash = hasher( key );
    size_t index = hash % capacity;
    return buckets + index;
}


template < typename key_t, typename value_t >
typename hashtable< key_t, value_t >::bucket*
                hashtable< key_t, value_t >::free_position( bucket* near )
{
    for ( bucket* free = near + 1; free < buckets + capacity; free += 1 )
    {
        if ( free->next == bucket::EMPTY )
            return free;
    }
    for ( bucket* free = near - 1; near >= buckets; free -= 1 )
    {
        if ( free->next == bucket::EMPTY )
            return free;
    }
    assert( ! "no free bucket" );
    return nullptr;
}
















#endif
