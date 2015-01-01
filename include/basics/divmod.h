//
//  divmod.h
//
//  Created by Edmund Kapusniak on 03/12/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef DIVMOD_H
#define DIVMOD_H


#include <math.h>


/*
    Floored (integer) division.
*/

float intdiv( float a, float b )
{
    return floor( a / b );
}


/*
    Modulus where the remainder has same sign as divisor.
*/

float mod( float a, float b )
{
    return a - floor( a / b ) * b;
}



#endif
