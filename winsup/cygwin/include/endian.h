/* endian.h

   Copyright 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <sys/config.h>

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

#ifndef __BYTE_ORDER
# define __BYTE_ORDER __LITTLE_ENDIAN
#endif

/*#ifdef  __USE_BSD*/
# define LITTLE_ENDIAN  __LITTLE_ENDIAN
# define BIG_ENDIAN     __BIG_ENDIAN
# define PDP_ENDIAN     __PDP_ENDIAN
# define BYTE_ORDER     __BYTE_ORDER
/*#endif*/

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define __LONG_LONG_PAIR(HI, LO) LO, HI
#elif __BYTE_ORDER == __BIG_ENDIAN
# define __LONG_LONG_PAIR(HI, LO) HI, LO
#endif
#endif /*_ENDIAN_H_*/

