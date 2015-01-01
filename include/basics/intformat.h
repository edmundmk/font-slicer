//
//  intformat.h
//
//  Created by Edmund Kapusniak on 13/02/2013.
//  Copyright (c) 2013 Edmund Kapusniak. All rights reserved.
//


#ifndef INTFORMAT_H
#define INTFORMAT_H


#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#ifdef _MSC_VER

#define PRIdSIZE        "ld"
#define PRIiSIZE        "li"
#define PRIoSIZE        "lo"
#define PRIuSIZE        "lu"
#define PRIxSIZE        "lx"
#define PRIXSIZE        "lX"

#define PRIdPTRDIFF     "ld"
#define PRIiPTRDIFF     "li"
#define PRIoPTRDIFF     "lo"
#define PRIuPTRDIFF     "lu"
#define PRIxPTRDIFF     "lx"
#define PRIXPTRIDFF     "lX"

#else

#define PRIdSIZE        "zd"
#define PRIiSIZE        "zi"
#define PRIoSIZE        "zo"
#define PRIuSIZE        "zu"
#define PRIxSIZE        "zx"
#define PRIXSIZE        "zX"

#define PRIdPTRDIFF     "td"
#define PRIiPTRDIFF     "ti"
#define PRIoPTRDIFF     "to"
#define PRIuPTRDIFF     "tu"
#define PRIxPTRDIFF     "tx"
#define PRIXPTRDIFF     "tX"

#endif


#endif
