/*
 *  $Id$
 */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H

/*
 * GCC wants type symmetry between size_t and ssize_t.
 * It supplies __SIZE_TYPE__, doesn't supply a corresponding __SSIZE_TYPE__,
 * so we have to guess on which type GCC wants ssize_t to be.
 * FIXME: GCC > 4.3.x supplies internal defines __SIZEOF_SIZE_T__ etc. which
 * could be applied here.
 */

#if defined(__i386__) || defined(__m32r__) || defined(__h8300__) || (defined(__arm__) && !defined(__ARM_EABI__)) || defined(__bfin__) || defined(__m68k__) || defined(__MICROBLAZE__)
#if defined(__H8300__)
typedef signed int _ssize_t;
#else
typedef long signed int _ssize_t;
#endif
#define __ssize_t_defined 1
#elif defined(__sparc__) && defined(__LP64__)
typedef long signed int _ssize_t;
#define __ssize_t_defined 1
#elif defined(__AVR__) || defined(__lm32__) || defined(__m32c__) || defined(__mips__) || defined(__moxie__) || defined(__PPC__) || defined(__sparc__) || defined(__sh__) || defined(__ARM_EABI__) \
  || defined(__v850__) || defined(__v851__)
typedef signed int _ssize_t;
#define __ssize_t_defined 1
#else
# error unsupported target
#endif

#include <machine/_default_types.h>

typedef __int32_t blksize_t;
typedef __int32_t blkcnt_t;

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
