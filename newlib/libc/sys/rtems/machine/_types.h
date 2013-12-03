/*
 *  $Id$
 */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H

/* This disables some conflicting type definitions in <machine/types.h> */
#define _HAVE_SYSTYPES

#include <machine/_default_types.h>
#include <stdint.h> /* For FreeBSD compatibility */

typedef __int32_t blksize_t;
typedef __int32_t blkcnt_t;

typedef __uint64_t __dev_t;
#define __dev_t_defined 1

#if defined(__arm__) || defined(__i386__) || defined(__m68k__) || defined(__mips__) || defined(__PPC__) || defined(__sparc__)
/* Use 64bit types */
typedef __int64_t _off_t;
#define __off_t_defined 1

typedef __int64_t _fpos_t;
#define __fpos_t_defined 1
#else
/* Use 32bit types */
typedef __int32_t _off_t;
#define __off_t_defined 1

typedef __int32_t _fpos_t;
#define __fpos_t_defined 1
#endif

typedef __uint32_t _mode_t;
#define __mode_t_defined 1

#endif
