//
//  make_unique.h
//
//  Created by Edmund Kapusniak on 17/02/2014.
//  Copyright (c) 2014 Edmund Kapusniak. Licensed under the GNU General Public
//  License, version 3. See the LICENSE file in the project root for full
//  license information.
//


#ifndef MAKE_UNIQUE_H
#define MAKE_UNIQUE_H


#include <memory>


namespace std
{

template < typename object_t, typename ... arguments_t >
unique_ptr< object_t > make_unique( arguments_t&& ... arguments )
{
    return unique_ptr< object_t >(
                new object_t( forward< arguments_t >( arguments ) ... ) );
}

}


#endif /* MAKE_UNIQUE */
