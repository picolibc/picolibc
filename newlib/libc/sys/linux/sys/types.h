/* libc/sys/linux/sys/types.h - The usual type zoo */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

/*
 * Okay, newlib has its own time_t and clock_t in libc/include/time.h
 * Since they're equivalent but not identical, we'll just skip the kernel's
 * definitions.
 */

#ifdef __time_t_defined
#define _TIME_T
#else
#define __time_t_defined
#endif

#ifdef __clock_t_defined
#define _CLOCK_T
#else
#define __clock_t_defined
#endif

#include <linux/types.h>

#endif
