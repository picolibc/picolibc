/* libc/sys/linux/include/alloca.h - Allocate memory on stack */

/* Written 2000 by Werner Almesberger */


#ifndef _NEWLIB_ALLOCA_H
#define _NEWLIB_ALLOCA_H

/* Simple, since we know that we use gcc */

#define alloca(size) __builtin_alloca(size)

#endif
