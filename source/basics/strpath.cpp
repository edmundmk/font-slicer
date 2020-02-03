//
//  strpath.cpp
//
//  Created by Edmund Kapusniak on 28/12/2012.
//  Copyright (c) 2012 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#include "strpath.h"
#include <assert.h>


#ifndef _WIN32
static const char SEPARATOR = '/';
#else
static const char SEPARATOR = '\\';
#endif

static const char PERIOD_SEPARATOR[] = { '.', SEPARATOR, '\0' };
static const char PERIOD_PERIOD_SEPARATOR[] = { '.', '.', SEPARATOR, '\0' };



static size_t find_slash( const std::string& path )
{
    size_t ifslash = path.rfind( '/' );
    size_t ibslash = path.rfind( '\\' );

    if ( ifslash != std::string::npos )
    {
        if ( ibslash != std::string::npos )
            return ifslash > ibslash ? ifslash + 1 : ibslash + 1;
        else
            return ifslash + 1;
    }
    else if ( ibslash != std::string::npos )
    {
        return ibslash + 1;
    }

    return 0;
}


static bool has_drive_letter( const std::string& path )
{
    if ( path.size() >= 2 )
    {
		// Normal Windows drive letters, eg C:
        return ( ( ( path[ 0 ] >= 'A' && path[ 0 ] <= 'Z' )
                        || ( path[ 0 ] >= 'a' && path[ 0 ] <= 'z' ) )
                  && path[ 1 ] == ':' )

 		// Treat UNC paths as a special kind of 'drive letter'.
		    || ( path[ 0 ] == '\\' && path[ 1 ] == '\\' )
		    || ( path[ 0 ] == '/' && path[ 1 ] == '/' );

    }
    else
    {
        return false;
    }
}


bool path_is_absolute( const std::string& path )
{
    size_t iroot = 0;
    if ( has_drive_letter( path ) )
        iroot = 2;

    return path.size() > iroot
        && ( path[ iroot ] == '/' || path[ iroot ] == '\\' );
}

bool path_is_relative( const std::string& path )
{
    return ! path_is_absolute( path );
}

bool path_is_directory( const std::string& path )
{
    return path.size() && ( path.back() == '/' || path.back() == '\\' );
}


std::string path_directory( const std::string& path )
{
    if ( path.size() == 0 )
        return std::string();
    size_t ifilename = find_slash( path );
    return path.substr( 0, ifilename );
}

std::string path_filename( const std::string& path )
{
    if ( path.size() == 0 )
        return std::string();
    size_t ifilename = find_slash( path );
    return path.substr( ifilename );
}

std::string path_bare_filename( const std::string& path )
{
    if ( path.size() == 0 )
        return std::string();
    size_t ifilename = find_slash( path );
    size_t iextension = path.find( '.', ifilename );
    if ( iextension != std::string::npos )
        return path.substr( ifilename, iextension - ifilename );
    else
        return path.substr( ifilename );
}

std::string path_extension( const std::string& path )
{
    size_t ifilename = find_slash( path );
    size_t iextension = path.find( '.', ifilename );
    if ( iextension != std::string::npos )
        return path.substr( iextension );
    else
        return std::string();
}

std::string path_bare_extension( const std::string& path )
{
    size_t ifilename = find_slash( path );
    size_t iextension = path.find( '.', ifilename );
    if ( iextension != std::string::npos )
        return path.substr( iextension + 1 );
    else
        return std::string();
}

std::string path_replace_extension(
        const std::string& path, const std::string& extension )
{
    size_t ifilename = find_slash( path );
    size_t iextension = path.find( '.', ifilename );
    if ( iextension != std::string::npos )
        return path.substr( 0, iextension ) + extension;
    else
        return path + extension;
}


std::string path_canonicalize( const std::string& path )
{
    std::string result;
    result.reserve( path.size() );

    enum ParseState
    {
        PARSE_ANY,
        PARSE_SEPARATOR,
        PARSE_PERIOD,
        PARSE_PERIOD_PERIOD
    };

    ParseState  state           = PARSE_SEPARATOR;
    size_t      num_components  = 0;
    size_t      i               = 0;

    if ( has_drive_letter( path ) )
    {
        result.append( path, 0, 2 );
        i = 2;
    }

    if ( path.size() > i && ( path[ i ] == '/' || path[ i ] == '\\' ) )
    {
        result.push_back( SEPARATOR );
        i += 1;
    }

    for ( ; i < path.size() || state == PARSE_PERIOD_PERIOD; ++i )
    {
        switch ( state )
        {
        case PARSE_ANY:
            if ( path[ i ] == '/' || path[ i ] == '\\' )
            {
                result.push_back( SEPARATOR );
                num_components += 1;
                state = PARSE_SEPARATOR;
            }
            else
            {
                result.push_back( path[ i ] );
            }
            break;

        case PARSE_SEPARATOR:
            if ( path[ i ] == '.' )
            {
                state = PARSE_PERIOD;
            }
            else if ( path[ i ] == '/' || path[ i ] == '\\' )
            {
                state = PARSE_SEPARATOR;
            }
            else
            {
                result.push_back( path[ i ] );
                state = PARSE_ANY;
            }
            break;

        case PARSE_PERIOD:
            if ( path[ i ] == '.' )
            {
                state = PARSE_PERIOD_PERIOD;
            }
            else if ( path[ i ] == '/' || path[ i ] == '\\' )
            {
                state = PARSE_SEPARATOR;
            }
            else
            {
                result.push_back( path[ i ] );
                state = PARSE_ANY;
            }
            break;

        case PARSE_PERIOD_PERIOD:
            if ( i >= path.size() || path[ i ] == '/' || path[ i ] == '\\' )
            {
                if ( num_components )
                {
                    assert( result.back() == SEPARATOR );
                    result.pop_back();
                    size_t islash = result.rfind( SEPARATOR );
                    if ( islash != std::string::npos )
                        result.erase( islash + 1 );
                    else
                        result.erase();
                    num_components -= 1;
                }
                else
                {
                    result.append( PERIOD_PERIOD_SEPARATOR );
                }

                state = PARSE_SEPARATOR;
            }
            else
            {
                result.append( ".." );
                result.push_back( path[ i ] );
                state = PARSE_ANY;
            }
        }
    }


    if ( ! result.size() )
        return PERIOD_SEPARATOR;
    else
        return result;
}


std::string path_relative(
            const std::string& directory, const std::string& target )
{
    std::string dpath = path_canonicalize( directory );
    std::string tpath = path_canonicalize( target );

    // Check for relative to current directory.
    if ( dpath.compare( PERIOD_SEPARATOR ) == 0 )
        return tpath;

    // Either both paths must be absolute or both relative to a common base.
    if ( path_is_absolute( dpath ) != path_is_absolute( tpath ) )
        return tpath;

    // Any drive letters must match or we can't do anything.
    if ( has_drive_letter( dpath ) &&
            ( ! has_drive_letter( tpath ) || dpath[ 0 ] != tpath[ 0 ] ) )
        return tpath;

    // Make sure the directory ends with a separator.
    if ( dpath.back() != SEPARATOR )
        dpath.push_back( SEPARATOR );

    // Check if target refers to directory.
    if ( tpath.back() == SEPARATOR )
    {
        if ( dpath.compare( tpath ) == 0 )
            return PERIOD_SEPARATOR;
    }
    else
    {
        if ( dpath.compare( 0, dpath.size() - 1, tpath ) == 0 )
            return PERIOD_SEPARATOR;
    }

    // Remove common prefix.
    size_t icommon = 0;
    for ( size_t i = 0; i < dpath.size() && i < tpath.size()
                                    && dpath[ i ] == tpath[ i ]; ++i )
    {
        if ( dpath[ i ] == SEPARATOR )
            icommon = i + 1;
    }

    dpath = dpath.substr( icommon );
    tpath = tpath.substr( icommon );
    assert( ! dpath.size() || dpath[ 0 ] != SEPARATOR );

    // Build result path.
    std::string result;

    // Each remaining path component in dpath must be undone by ../
    size_t islash = dpath.find( SEPARATOR );
    while ( islash != std::string::npos )
    {
        result.append( PERIOD_PERIOD_SEPARATOR );
        islash = dpath.find( SEPARATOR, islash + 1 );
    }

    // Add the remainder of tpath.
    result += tpath;

    return result;
}


std::string path_join( const std::string& a, const std::string& b )
{
    if ( ! a.size() || path_is_absolute( b ) )
        return b;

    return path_canonicalize( a + '/' + b );
}



