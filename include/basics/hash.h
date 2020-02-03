//
//  hash.h
//
//  Created by Edmund Kapusniak on 28/07/2013.
//  Copyright (c) 2013 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef HASH_H
#define HASH_H


#include <stdlib.h>
#include <stdint.h>


/*
    Our hash function is SpookyHash v2.  Used to use MurmurHash3 32 but
    Spooky has the advantages of a larger key length and incremental hashing.
*/


class SpookyHash;


typedef uint32_t    hash32_t;
typedef uint64_t    hash64_t;
typedef uintptr_t   hash_t;


hash32_t    hash32( const char* string );
hash32_t    hash32( const void* data, size_t size );
hash64_t    hash64( const char* string );
hash64_t    hash64( const void* data, size_t size );
hash_t      hash( const char* string );
hash_t      hash( const void* data, size_t size );


class hash_context
{
public:

    hash_context();
    ~hash_context();

    void        data( uint8_t i );
    void        data( uint16_t i );
    void        data( uint32_t i );
    void        data( uint64_t i );
    void        data( int8_t i );
    void        data( int16_t i );
    void        data( int32_t i );
    void        data( int64_t i );
    void        data( float f );
    void        data( double f );
    void        data( const char* string );
    void        data( const void* data, size_t size );

    hash32_t    hash32();
    hash64_t    hash64();


private:

    SpookyHash* context;

};



#endif /* HASH_H */

