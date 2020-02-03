//
//  exception.h
//  toolbox
//
//  Created by Edmund Kapusniak on 01/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef EXCEPTION_H
#define EXCEPTION_H


#include <stdarg.h>
#include <exception>
#include "stringf.h"


class _exception : public std::exception
{
public:

    virtual const char* what() const throw();

protected:

    std::string message;

};



inline const char* _exception::what() const throw()
{
    return message.c_str();
}



#define EXCEPTION( x ) \
    class x : public _exception \
    { \
    public: \
        x( const char* format, ... ) \
        { \
            va_list arguments; \
            va_start( arguments, format ); \
            message = vstringf( format, arguments ); \
            va_end( arguments ); \
        } \
    };



#endif
