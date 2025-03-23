/*
  Copyright (c) 1982, 1986, 1993
  The Regents of the University of California.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
*/
#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__

#include <machine/ieeefp.h>  /* floating point macros */
#include <sys/features.h>	/* POSIX defs */
#include <float.h>

#ifdef __aarch64__
#define MALLOC_ALIGNMENT 16
#endif

/* exceptions first */
#if defined(__H8500__) || defined(__W65__)
#define __SMALL_BITFIELDS
/* ???  This conditional is true for the h8500 and the w65, defining H8300
   in those cases probably isn't the right thing to do.  */
#define H8300 1
#endif

/* 16 bit integer machines */
#if defined(__Z8001__) || defined(__Z8002__) || defined(__H8500__) || defined(__W65__) || defined (__mn10200__) || defined (__AVR__) || defined (__MSP430__)

#undef INT_MAX
#undef UINT_MAX
#define INT_MAX 32767
#define UINT_MAX 65535
#endif

#if defined (__H8300__) || defined (__H8300H__) || defined(__H8300S__) || defined (__H8300SX__)
#define __SMALL_BITFIELDS
#define H8300 1
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX __INT_MAX__
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#endif

#if (defined(__CR16__) || defined(__CR16C__) ||defined(__CR16CP__))
#ifndef __INT32__
#define __SMALL_BITFIELDS      
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX 32767
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#else /* INT32 */
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX 2147483647
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#endif /* INT32 */

#endif /* CR16C */

#if defined (__xc16x__) || defined (__xc16xL__) || defined (__xc16xS__)
#define __SMALL_BITFIELDS
#endif

#ifdef __W65__
#define __SMALL_BITFIELDS
#endif

#if defined(__D10V__)
#define __SMALL_BITFIELDS
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX __INT_MAX__
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#define _POINTER_INT short
#endif

#if defined(__mc68hc11__) || defined(__mc68hc12__) || defined(__mc68hc1x__)
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX __INT_MAX__
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#define _POINTER_INT short
#endif

#ifdef __i386__
#if defined(__linux__) || defined(__RDOS__)
/* we want the reentrancy structure to be returned by a function */
#define HAVE_GETDATE
#ifndef __LARGE64_FILES
#define __LARGE64_FILES 1
#endif
/* we use some glibc header files so turn on glibc large file feature */
#define _LARGEFILE64_SOURCE 1
#endif
#endif

#ifdef __mn10200__
#define __SMALL_BITFIELDS
#endif

#ifdef __AVR__
#define __SMALL_BITFIELDS
#define _POINTER_INT short
#endif

#if defined (__MICROBLAZE__) && !defined(__rtems__)
/* Xilinx XMK uses Unix98 mutex */
#ifdef __XMK__
#define _UNIX98_THREAD_MUTEX_ATTRIBUTES
#endif
#endif

#ifdef __xstormy16__
#define __SMALL_BITFIELDS
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX __INT_MAX__
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#define MALLOC_ALIGNMENT 8
#define _POINTER_INT short
#define __BUFSIZ__ 16
#endif

#if defined __MSP430__

#define __BUFSIZ__ 256
#define __SMALL_BITFIELDS

#ifdef __MSP430X_LARGE__
#define _POINTER_INT __int20
#else
#define _POINTER_INT int
#endif
#endif

#ifdef __m32c__
#define __SMALL_BITFIELDS
#undef INT_MAX
#undef UINT_MAX
#define INT_MAX __INT_MAX__
#define UINT_MAX (__INT_MAX__ * 2U + 1)
#define MALLOC_ALIGNMENT 8
#if defined(__r8c_cpu__) || defined(__m16c_cpu__)
#define _POINTER_INT short
#else
#define _POINTER_INT long
#endif
#define __BUFSIZ__ 16
#endif /* __m32c__ */

#ifdef __SPU__
#define MALLOC_ALIGNMENT 16
#define __CUSTOM_FILE_IO__
#endif

/* This block should be kept in sync with GCC's limits.h.  The point
   of having these definitions here is to not include limits.h, which
   would pollute the user namespace, while still using types of the
   the correct widths when deciding how to define __int32_t and
   __int64_t.  */
#ifndef __INT_MAX__
# ifdef INT_MAX
#  define __INT_MAX__ INT_MAX
# else
#  define __INT_MAX__ 2147483647
# endif
#endif

#ifndef __LONG_MAX__
# ifdef LONG_MAX
#  define __LONG_MAX__ LONG_MAX
# else
#  if defined (__alpha__) || (defined (__sparc__) && defined(__arch64__)) \
    || defined (__sparcv9)
#   define __LONG_MAX__ 9223372036854775807L
#  else
#   define __LONG_MAX__ 2147483647L
#  endif /* __alpha__ || sparc64 */
# endif
#endif
/* End of block that should be kept in sync with GCC's limits.h.  */

#ifndef _POINTER_INT
#define _POINTER_INT long
#endif

#undef __RAND_MAX
#if __INT_MAX__ == 32767
#define __RAND_MAX 32767
#else
#define __RAND_MAX 0x7fffffff
#endif


#if defined(__rtems__)
#define __FILENAME_MAX__ 255
#endif

#ifndef __WCHAR_MAX__
#if __INT_MAX__ == 32767 || defined (_WIN32)
#define __WCHAR_MAX__ 0xffffu
#endif
#endif

#ifdef __THREAD_LOCAL_STORAGE
#if (defined(__cplusplus) && (__cplusplus) >= 201103L) ||               \
    (defined(__STDC_VERSION__) && (__STDC_VERSION__) >= 202311L)
#define __THREAD_LOCAL thread_local
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__) >= 201112L
#define __THREAD_LOCAL _Thread_local
#else
#define __THREAD_LOCAL __thread
#endif
#else
#define __THREAD_LOCAL
#endif

#ifndef __MB_CAPABLE
/* Make sure all of these are disabled if multi-byte is disabled*/
#undef __MB_EXTENDED_CHARSETS_ALL
#undef __MB_EXTENDED_CHARSETS_ANY
#undef __MB_EXTENDED_CHARSETS_UCS
#undef __MB_EXTENDED_CHARSETS_ISO
#undef __MB_EXTENDED_CHARSETS_WINDOWS
#undef __MB_EXTENDED_CHARSETS_JIS
#endif

/* If __MB_EXTENDED_CHARSETS_ALL is set, we want all of the extended
   charsets.  The extended charsets add a few functions and a couple
   of tables of a few K each. */
#ifdef __MB_EXTENDED_CHARSETS_ALL
#define __MB_EXTENDED_CHARSETS_UCS 1
#define __MB_EXTENDED_CHARSETS_ISO 1
#define __MB_EXTENDED_CHARSETS_WINDOWS 1
#define __MB_EXTENDED_CHARSETS_JIS 1
#endif

#if defined(__MB_EXTENDED_CHARSETS_ISO) ||       \
    defined(__MB_EXTENDED_CHARSETS_WINDOWS) ||   \
    defined(__MB_EXTENDED_CHARSETS_JIS)
#define __MB_EXTENDED_CHARSETS_NON_UNICODE
#endif

#if defined(__MB_EXTENDED_CHARSETS_UCS) ||       \
    defined(__MB_EXTENDED_CHARSETS_NON_UNICODE)
#define __MB_EXTENDED_CHARSETS_ANY
#endif

/* Figure out if the compiler supports the long double type. */
#ifdef __SIZEOF_LONG_DOUBLE__
#define __HAVE_LONG_DOUBLE
#endif

/* Figure out if long double is the same size as double. If the system
 * doesn't provide long double, then those values will be undefined
 * and cpp will substitute 0 for them in the test
 */

#if LDBL_MANT_DIG == DBL_MANT_DIG && LDBL_MIN_EXP == DBL_MIN_EXP &&     \
    LDBL_MAX_EXP == DBL_MAX_EXP
#define _LDBL_EQ_DBL
#endif

/* Newlib doesn't fully support long double math functions so far.
   On platforms where long double equals double the long double functions
   simply call the double functions.  On Cygwin the long double functions
   are implemented independently from newlib to be able to use optimized
   assembler functions despite using the Microsoft x86_64 ABI. */
#if defined (_LDBL_EQ_DBL) || defined (__CYGWIN__) || (defined(__HAVE_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ <= 8) || (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113)
#define __HAVE_LONG_DOUBLE_MATH
#endif

#endif /* __SYS_CONFIG_H__ */
