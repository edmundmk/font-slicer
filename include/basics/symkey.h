//
//  symkey.h
//
//  Created 3rd October 2013.
//  Copyright (c) Edmund Kapusniak 2013.  Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef SYMKEY_H
#define SYMKEY_H


#include <string>
#include <functional>
#include <hash.h>


/*
    symkeys can be used in place of string keys for hash tables, when the
    string memory is managed elsewhere (perhaps as part of the corresponding
    value).  It stores the already-calculated hash.
*/


class symkey
{
public:

    symkey();
    symkey( const char* s );
    symkey( const char* s, size_t size );
    symkey( hash32_t hash, const char* s, size_t size );

    explicit operator bool () const;
    operator std::string () const;
    const char* c_str() const;
    hash32_t hash() const;
    size_t size() const;


private:

    friend struct std::hash< symkey >;
    friend bool operator == ( const symkey& a, const symkey& b );

    hash32_t    shash;
    const char* sname;
    size_t      ssize;

};



inline symkey::symkey()
    :   shash( 0 )
    ,   sname( NULL )
    ,   ssize( NULL )
{
}

inline symkey::symkey( const char* s )
    :   shash( hash32( s ) )
    ,   sname( s )
    ,   ssize( strlen( s ) )
{
}

inline symkey::symkey( const char* s, size_t size )
    :   shash( hash32( s, size ) )
    ,   sname( s )
    ,   ssize( size )
{
}

inline symkey::symkey( hash32_t hash, const char* s, size_t size )
    :   shash( hash )
    ,   sname( s )
    ,   ssize( size )
{
}


inline symkey::operator bool () const
{
    return sname != NULL;
}

inline symkey::operator std::string () const
{
    return std::string( sname, ssize );
}

inline const char* symkey::c_str() const
{
    return sname;
}

inline hash32_t symkey::hash() const
{
    return shash;
}

inline size_t symkey::size() const
{
    return ssize;
}



namespace std
{
template <> struct hash< symkey >
{
    inline size_t operator () ( const symkey& s ) const
    {
        return s.shash;
    }
};
}


inline bool operator == ( const symkey& a, const symkey& b )
{
    return a.shash == b.shash && a.ssize == b.ssize
                    && memcmp( a.sname, b.sname, a.ssize ) == 0;
}

inline bool operator != ( const symkey& a, const symkey& b )
{
    return !( a == b );
}



#endif /* SYMKEY_H */

