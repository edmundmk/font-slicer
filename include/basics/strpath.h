//
//  strpath.h
//
//  Created by Edmund Kapusniak on 28/12/2012.
//  Copyright (c) 2012 Edmund Kapusniak. All rights reserved.
//


#ifndef STRPATH_H
#define STRPATH_H


#include <string>


bool path_is_absolute( const std::string& path );
bool path_is_relative( const std::string& path );
bool path_is_directory( const std::string& path );

std::string path_directory( const std::string& path );
std::string path_filename( const std::string& path );
std::string path_bare_filename( const std::string& path );
std::string path_extension( const std::string& path );
std::string path_bare_extension( const std::string& path );
std::string path_replace_extension(
        const std::string& path, const std::string& extension );

std::string path_canonicalize( const std::string& path );
std::string path_relative(
        const std::string& directory, const std::string& target );
std::string path_join( const std::string& a, const std::string& b );


#endif /* STRPATH_H */

