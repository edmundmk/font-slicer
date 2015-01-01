//
//  stringf.h
//
//  Created by Edmund Kapusniak on 29/09/2012.
//  Copyright (c) 2012 Edmund Kapusniak. All rights reserved.
//


#ifndef STRINGF_H
#define STRINGF_H


#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <string>


#ifdef _MSC_VER

#define va_copy( vlist, arguments ) vlist = arguments

inline int _msc_vsnprintf( char* str, size_t size, const char* format, va_list ap )
{
    va_list vlist;
    va_copy( vlist, ap );
    int result = vsnprintf( str, size, format, vlist );
    if ( result == -1 )
    {
        va_copy( vlist, ap );
        result = _vscprintf( format, vlist );
    }
    return result;
}

inline int _msc_snprintf( char* str, size_t size, const char* format, ... )
{
    va_list arguments;
    va_start( arguments, format );
    int result = _msc_vsnprintf( str, size, format, arguments );
    va_end( arguments )
    return result;
}

#define vsnprintf _msc_vsnprintf
#define snprintf _msc_snprintf

#endif


std::string stringf( const char* format, ... );
std::string vstringf( const char* format, va_list arguments );


inline std::string stringf( const char* format, ... )
{
    va_list arguments;
    va_start( arguments, format );
    std::string result = vstringf( format, arguments );
    va_end( arguments );
    return result;
}


inline std::string vstringf( const char* format, va_list arguments )
{
    va_list vlist;
 
    va_copy( vlist, arguments );
    int bufsize = vsnprintf( NULL, 0, format, vlist );
    va_end( vlist );
    
    std::vector< char > buffer;
    buffer.resize( bufsize + 1 );
    
    va_copy( vlist, arguments );
    vsnprintf( buffer.data(), buffer.size(), format, vlist );
    va_end( vlist );
    
    return std::string( buffer.data() );

}



#endif /* STRINGF_H */

