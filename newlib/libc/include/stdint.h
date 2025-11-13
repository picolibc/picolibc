/*
 * Copyright (c) 2004, 2005 by
 * Ralf Corsepius, Ulm/Germany. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#ifndef _STDINT_H
#define _STDINT_H

#include <sys/cdefs.h>
#include <machine/_default_types.h>
#include <sys/_stdint.h>
#include <sys/_intsup.h>

_BEGIN_STD_C

#if __ISO_C_VISIBLE >= 2023
#define __STDC_VERSION_STDINT_H__ 0x202311L
#endif

#ifdef ___int_least8_t_defined
typedef __int_least8_t int_least8_t;
typedef __uint_least8_t uint_least8_t;
#define __int_least8_t_defined 1
#endif

#ifdef ___int_least16_t_defined
typedef __int_least16_t int_least16_t;
typedef __uint_least16_t uint_least16_t;
#define __int_least16_t_defined 1
#endif

#ifdef ___int_least32_t_defined
typedef __int_least32_t int_least32_t;
typedef __uint_least32_t uint_least32_t;
#define __int_least32_t_defined 1
#endif

#ifdef ___int_least64_t_defined
typedef __int_least64_t int_least64_t;
typedef __uint_least64_t uint_least64_t;
#define __int_least64_t_defined 1
#endif

/*
 * Fastest minimum-width integer types
 *
 * Assume int to be the fastest type for all types with a width 
 * less than __INT_MAX__ rsp. INT_MAX
 */
#ifdef __INT_FAST8_TYPE__
  typedef __INT_FAST8_TYPE__ int_fast8_t;
  typedef __UINT_FAST8_TYPE__ uint_fast8_t;
#define __int_fast8_t_defined 1
#elif __STDINT_EXP(INT_MAX) >= 0x7f
  typedef signed int int_fast8_t;
  typedef unsigned int uint_fast8_t;
#define __int_fast8_t_defined 1
#endif

#ifdef __INT_FAST16_TYPE__
  typedef __INT_FAST16_TYPE__ int_fast16_t;
  typedef __UINT_FAST16_TYPE__ uint_fast16_t;
#define __int_fast16_t_defined 1
#elif __STDINT_EXP(INT_MAX) >= 0x7fff
  typedef signed int int_fast16_t;
  typedef unsigned int uint_fast16_t;
#define __int_fast16_t_defined 1
#endif

#ifdef __INT_FAST32_TYPE__
  typedef __INT_FAST32_TYPE__ int_fast32_t;
  typedef __UINT_FAST32_TYPE__ uint_fast32_t;
#define __int_fast32_t_defined 1
#elif __STDINT_EXP(INT_MAX) >= 0x7fffffff
  typedef signed int int_fast32_t;
  typedef unsigned int uint_fast32_t;
#define __int_fast32_t_defined 1
#endif

#ifdef __INT_FAST64_TYPE__
  typedef __INT_FAST64_TYPE__ int_fast64_t;
  typedef __UINT_FAST64_TYPE__ uint_fast64_t;
#define __int_fast64_t_defined 1
#elif __STDINT_EXP(INT_MAX) > 0x7fffffff
  typedef signed int int_fast64_t;
  typedef unsigned int uint_fast64_t;
#define __int_fast64_t_defined 1
#endif

/*
 * Fall back to [u]int_least<N>_t for [u]int_fast<N>_t types
 * not having been defined, yet.
 * Leave undefined, if [u]int_least<N>_t should not be available.
 */
#if !__int_fast8_t_defined
#if __int_least8_t_defined
  typedef int_least8_t int_fast8_t;
  typedef uint_least8_t uint_fast8_t;
#define __int_fast8_t_defined 1
#endif
#endif

#if !__int_fast16_t_defined
#if __int_least16_t_defined
  typedef int_least16_t int_fast16_t;
  typedef uint_least16_t uint_fast16_t;
#define __int_fast16_t_defined 1
#endif
#endif

#if !__int_fast32_t_defined
#if __int_least32_t_defined
  typedef int_least32_t int_fast32_t;
  typedef uint_least32_t uint_fast32_t;
#define __int_fast32_t_defined 1
#endif
#endif

#if !__int_fast64_t_defined
#if __int_least64_t_defined
  typedef int_least64_t int_fast64_t;
  typedef uint_least64_t uint_fast64_t;
#define __int_fast64_t_defined 1
#endif
#endif

#ifdef __INTPTR_TYPE__
#define INTPTR_MIN (-__INTPTR_MAX__ - 1)
#define INTPTR_MAX (__INTPTR_MAX__)
#define UINTPTR_MAX (__UINTPTR_MAX__)
#elif defined(__PTRDIFF_TYPE__)
#define INTPTR_MAX PTRDIFF_MAX
#define INTPTR_MIN PTRDIFF_MIN
#ifdef __UINTPTR_MAX__
#define UINTPTR_MAX (__UINTPTR_MAX__)
#else
#define UINTPTR_MAX (2UL * PTRDIFF_MAX + 1)
#endif
#else
/*
 * Fallback to hardcoded values, 
 * should be valid on cpu's with 32bit int/32bit void*
 */
#define INTPTR_MAX (__STDINT_EXP(LONG_MAX))
#define INTPTR_MIN (-__STDINT_EXP(LONG_MAX) - 1)
#define UINTPTR_MAX (__STDINT_EXP(LONG_MAX) * 2UL + 1)
#endif

/* Limits of Specified-Width Integer Types */

#ifdef __INT8_MAX__
#define INT8_MIN (-__INT8_MAX__ - 1)
#define INT8_MAX (__INT8_MAX__)
#define UINT8_MAX (__UINT8_MAX__)
#elif defined(__int8_t_defined)
#define INT8_MIN 	(-128)
#define INT8_MAX 	 (127)
#define UINT8_MAX 	 (255)
#endif

#ifdef __INT_LEAST8_MAX__
#define INT_LEAST8_MIN (-__INT_LEAST8_MAX__ - 1)
#define INT_LEAST8_MAX (__INT_LEAST8_MAX__)
#define UINT_LEAST8_MAX (__UINT_LEAST8_MAX__)
#elif defined(__int_least8_t_defined)
#define INT_LEAST8_MIN 	(-128)
#define INT_LEAST8_MAX 	 (127)
#define UINT_LEAST8_MAX	 (255)
#else
#error required type int_least8_t missing
#endif

#ifdef __INT16_MAX__
#define INT16_MIN (-__INT16_MAX__ - 1)
#define INT16_MAX (__INT16_MAX__)
#define UINT16_MAX (__UINT16_MAX__)
#elif defined(__int16_t_defined)
#define INT16_MIN 	(-32768)
#define INT16_MAX 	 (32767)
#define UINT16_MAX 	 (65535)
#endif

#ifdef __INT_LEAST16_MAX__
#define INT_LEAST16_MIN (-__INT_LEAST16_MAX__ - 1)
#define INT_LEAST16_MAX (__INT_LEAST16_MAX__)
#define UINT_LEAST16_MAX (__UINT_LEAST16_MAX__)
#elif defined(__int_least16_t_defined)
#define INT_LEAST16_MIN	(-32768)
#define INT_LEAST16_MAX	 (32767)
#define UINT_LEAST16_MAX (65535)
#else
#error required type int_least16_t missing
#endif

#ifdef __INT32_MAX__
#define INT32_MIN (-__INT32_MAX__ - 1)
#define INT32_MAX (__INT32_MAX__)
#define UINT32_MAX (__UINT32_MAX__)
#elif defined(__int32_t_defined)
#if defined (_INT32_EQ_LONG)
#define INT32_MIN 	 (-2147483647L-1)
#define INT32_MAX 	 (2147483647L)
#define UINT32_MAX       (4294967295UL)
#else
#define INT32_MIN 	 (-2147483647-1)
#define INT32_MAX 	 (2147483647)
#define UINT32_MAX       (4294967295U)
#endif
#endif

#ifdef __INT_LEAST32_MAX__
#define INT_LEAST32_MIN (-__INT_LEAST32_MAX__ - 1)
#define INT_LEAST32_MAX (__INT_LEAST32_MAX__)
#define UINT_LEAST32_MAX (__UINT_LEAST32_MAX__)
#elif defined(__int_least32_t_defined)
#if defined (_INT32_EQ_LONG)
#define INT_LEAST32_MIN  (-2147483647L-1)
#define INT_LEAST32_MAX  (2147483647L)
#define UINT_LEAST32_MAX (4294967295UL)
#else
#define INT_LEAST32_MIN  (-2147483647-1)
#define INT_LEAST32_MAX  (2147483647)
#define UINT_LEAST32_MAX (4294967295U)
#endif
#else
#error required type int_least32_t missing
#endif

#ifdef __INT64_MAX__
#define INT64_MIN (-__INT64_MAX__ - 1)
#define INT64_MAX (__INT64_MAX__)
#define UINT64_MAX (__UINT64_MAX__)
#elif defined(__int64_t_defined)
#if __have_long64
#define INT64_MIN 	(-9223372036854775807L-1L)
#define INT64_MAX 	 (9223372036854775807L)
#define UINT64_MAX 	(18446744073709551615U)
#elif __have_longlong64
#define INT64_MIN 	(-9223372036854775807LL-1LL)
#define INT64_MAX 	 (9223372036854775807LL)
#define UINT64_MAX 	(18446744073709551615ULL)
#endif
#endif

#ifdef __INT_LEAST64_MAX__
#define INT_LEAST64_MIN (-__INT_LEAST64_MAX__ - 1)
#define INT_LEAST64_MAX (__INT_LEAST64_MAX__)
#define UINT_LEAST64_MAX (__UINT_LEAST64_MAX__)
#elif defined(__int_least64_t_defined)
#if __have_long64
#define INT_LEAST64_MIN  (-9223372036854775807L-1L)
#define INT_LEAST64_MAX  (9223372036854775807L)
#define UINT_LEAST64_MAX (18446744073709551615U)
#elif __have_longlong64
#define INT_LEAST64_MIN  (-9223372036854775807LL-1LL)
#define INT_LEAST64_MAX  (9223372036854775807LL)
#define UINT_LEAST64_MAX (18446744073709551615ULL)
#endif
#endif

#ifdef __INT_FAST8_MAX__
#define INT_FAST8_MIN (-__INT_FAST8_MAX__ - 1)
#define INT_FAST8_MAX (__INT_FAST8_MAX__)
#define UINT_FAST8_MAX (__UINT_FAST8_MAX__)
#elif defined(__int_fast8_t_defined)
#if __STDINT_EXP(INT_MAX) >= 0x7f
#define INT_FAST8_MIN	(-__STDINT_EXP(INT_MAX)-1)
#define INT_FAST8_MAX	(__STDINT_EXP(INT_MAX))
#define UINT_FAST8_MAX	(__STDINT_EXP(INT_MAX)*2U+1U)
#else
#define INT_FAST8_MIN	INT_LEAST8_MIN
#define INT_FAST8_MAX	INT_LEAST8_MAX
#define UINT_FAST8_MAX	UINT_LEAST8_MAX
#endif
#endif

#ifdef __INT_FAST16_MAX__
#define INT_FAST16_MIN (-__INT_FAST16_MAX__ - 1)
#define INT_FAST16_MAX (__INT_FAST16_MAX__)
#define UINT_FAST16_MAX (__UINT_FAST16_MAX__)
#elif defined(__int_fast16_t_defined)
#if __STDINT_EXP(INT_MAX) >= 0x7fff
#define INT_FAST16_MIN	(-__STDINT_EXP(INT_MAX)-1)
#define INT_FAST16_MAX	(__STDINT_EXP(INT_MAX))
#define UINT_FAST16_MAX	(__STDINT_EXP(INT_MAX)*2U+1U)
#else
#define INT_FAST16_MIN	INT_LEAST16_MIN
#define INT_FAST16_MAX	INT_LEAST16_MAX
#define UINT_FAST16_MAX	UINT_LEAST16_MAX
#endif
#endif

#ifdef __INT_FAST32_MAX__
#define INT_FAST32_MIN (-__INT_FAST32_MAX__ - 1)
#define INT_FAST32_MAX (__INT_FAST32_MAX__)
#define UINT_FAST32_MAX (__UINT_FAST32_MAX__)
#elif defined(__int_fast32_t_defined)
#if __STDINT_EXP(INT_MAX) >= 0x7fffffff
#define INT_FAST32_MIN	(-__STDINT_EXP(INT_MAX)-1)
#define INT_FAST32_MAX	(__STDINT_EXP(INT_MAX))
#define UINT_FAST32_MAX	(__STDINT_EXP(INT_MAX)*2U+1U)
#else
#define INT_FAST32_MIN	INT_LEAST32_MIN
#define INT_FAST32_MAX	INT_LEAST32_MAX
#define UINT_FAST32_MAX	UINT_LEAST32_MAX
#endif
#endif

#ifdef __INT_FAST64_MAX__
#define INT_FAST64_MIN (-__INT_FAST64_MAX__ - 1)
#define INT_FAST64_MAX (__INT_FAST64_MAX__)
#define UINT_FAST64_MAX (__UINT_FAST64_MAX__)
#elif defined(__int_fast64_t_defined)
#if __STDINT_EXP(INT_MAX) > 0x7fffffff
#define INT_FAST64_MIN	(-__STDINT_EXP(INT_MAX)-1)
#define INT_FAST64_MAX	(__STDINT_EXP(INT_MAX))
#define UINT_FAST64_MAX	(__STDINT_EXP(INT_MAX)*2U+1U)
#else
#define INT_FAST64_MIN	INT_LEAST64_MIN
#define INT_FAST64_MAX	INT_LEAST64_MAX
#define UINT_FAST64_MAX	UINT_LEAST64_MAX
#endif
#endif

#ifdef __INTMAX_MAX__
#define INTMAX_MAX (__INTMAX_MAX__)
#define INTMAX_MIN (-INTMAX_MAX - 1)
#elif defined(__INTMAX_TYPE__)
/* All relevant GCC versions prefer long to long long for intmax_t.  */
#define INTMAX_MAX INT64_MAX
#define INTMAX_MIN INT64_MIN
#endif

#ifdef __UINTMAX_MAX__
#define UINTMAX_MAX (__UINTMAX_MAX__)
#elif defined(__UINTMAX_TYPE__)
/* All relevant GCC versions prefer long to long long for intmax_t.  */
#define UINTMAX_MAX UINT64_MAX
#endif

/* This must match size_t in stddef.h, currently long unsigned int */
#ifdef __SIZE_MAX__
#define SIZE_MAX (__SIZE_MAX__)
#else
#define SIZE_MAX (__STDINT_EXP(LONG_MAX) * 2UL + 1)
#endif

/* This must match sig_atomic_t in <signal.h> (currently int) */
#define SIG_ATOMIC_MIN (-__STDINT_EXP(INT_MAX) - 1)
#define SIG_ATOMIC_MAX (__STDINT_EXP(INT_MAX))

/* This must match ptrdiff_t  in <stddef.h> (currently long int) */
#ifdef __PTRDIFF_MAX__
#define PTRDIFF_MAX (__PTRDIFF_MAX__)
#else
#define PTRDIFF_MAX (__STDINT_EXP(LONG_MAX))
#endif
#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)

/* This must match definition in <wchar.h> */
#ifndef WCHAR_MIN
#ifdef __WCHAR_MIN__
#define WCHAR_MIN (__WCHAR_MIN__)
#elif defined(__WCHAR_UNSIGNED__) || (L'\0' - 1 > 0)
#define WCHAR_MIN (0 + L'\0')
#else
#define WCHAR_MIN (-0x7fffffff - 1 + L'\0')
#endif
#endif

/* This must match definition in <wchar.h> */
#ifndef WCHAR_MAX
#ifdef __WCHAR_MAX__
#define WCHAR_MAX (__WCHAR_MAX__)
#elif defined(__WCHAR_UNSIGNED__) || (L'\0' - 1 > 0)
#define WCHAR_MAX (0xffffffffu + L'\0')
#else
#define WCHAR_MAX (0x7fffffff + L'\0')
#endif
#endif

/* wint_t is unsigned int on almost all GCC targets.  */
#ifdef __WINT_MAX__
#define WINT_MAX (__WINT_MAX__)
#else
#define WINT_MAX (__STDINT_EXP(INT_MAX) * 2U + 1U)
#endif
#ifdef __WINT_MIN__
#define WINT_MIN (__WINT_MIN__)
#else
#define WINT_MIN (0U)
#endif

/** Macros for minimum-width integer constant expressions */
#ifdef __INT8_C
#define INT8_C(x) __INT8_C(x)
#define UINT8_C(x) __UINT8_C(x)
#else
#define INT8_C(x)	x
#if __STDINT_EXP(INT_MAX) > 0x7f
#define UINT8_C(x)	x
#else
#define UINT8_C(x)	x##U
#endif
#endif

#ifdef __INT16_C
#define INT16_C(x) __INT16_C(x)
#define UINT16_C(x) __UINT16_C(x)
#else
#define INT16_C(x)	x
#if __STDINT_EXP(INT_MAX) > 0x7fff
#define UINT16_C(x)	x
#else
#define UINT16_C(x)	x##U
#endif
#endif

#ifdef __INT32_C
#define INT32_C(x) __INT32_C(x)
#define UINT32_C(x) __UINT32_C(x)
#else
#if defined (_INT32_EQ_LONG)
#define INT32_C(x)	x##L
#define UINT32_C(x)	x##UL
#else
#define INT32_C(x)	x
#define UINT32_C(x)	x##U
#endif
#endif

#ifdef __INT64_C
#define INT64_C(x) __INT64_C(x)
#define UINT64_C(x) __UINT64_C(x)
#else
#if __int64_t_defined
#if __have_long64
#define INT64_C(x)	x##L
#define UINT64_C(x)	x##UL
#else
#define INT64_C(x)	x##LL
#define UINT64_C(x)	x##ULL
#endif
#endif
#endif

/** Macros for greatest-width integer constant expression */
#ifdef __INTMAX_C
#define INTMAX_C(x) __INTMAX_C(x)
#define UINTMAX_C(x) __UINTMAX_C(x)
#else
#if __have_long64
#define INTMAX_C(x)	x##L
#define UINTMAX_C(x)	x##UL
#else
#define INTMAX_C(x)	x##LL
#define UINTMAX_C(x)	x##ULL
#endif
#endif

#if __STDC_WANT_LIB_EXT1__ == 1
#ifndef __STDC_LIB_EXT1__
#define __STDC_LIB_EXT1__ 1
#endif

  // could be defined by the user
#ifndef RSIZE_MAX
#define RSIZE_MAX SIZE_MAX
#endif
#endif

#if __ISO_C_VISIBLE >= 2023

/* define INT8_WIDTH and UINT8_WIDTH if possible */
#ifdef __int8_t_defined
#ifdef __INT8_WIDTH__
#define INT8_WIDTH __INT8_WIDTH__
#elif defined __UINT8_WIDTH__
#define INT8_WIDTH __UINT8_WIDTH__
#else
#define INT8_WIDTH 8
#endif
#ifdef __UINT8_WIDTH__
#define UINT8_WIDTH __UINT8_WIDTH__
#elif defined __INT8_WIDTH__
#define UINT8_WIDTH __INT8_WIDTH__
#else
#define UINT8_WIDTH 8
#endif
#endif

/* define INT16_WIDTH and UINT16_WIDTH if possible */
#ifdef _int16_t_defined
#ifdef __INT16_WIDTH__
#define INT16_WIDTH __INT16_WIDTH__
#elif defined __UINT16_WIDTH__
#define INT16_WIDTH __UINT16_WIDTH__
#else
#define INT16_WIDTH 16
#endif
#ifdef __UINT16_WIDTH__
#define UINT16_WIDTH __UINT16_WIDTH__
#elif defined __INT16_WIDTH__
#define UINT16_WIDTH __INT16_WIDTH__
#else
#define UINT16_WIDTH 16
#endif
#endif

/* define INT32_WIDTH and UINT32_WIDTH if possible */
#ifdef __int32_t_defined
#ifdef __INT32_WIDTH__
#define INT32_WIDTH __INT32_WIDTH__
#elif defined __UINT32_WIDTH__
#define INT32_WIDTH __UINT32_WIDTH__
#else
#define INT32_WIDTH 32
#endif
#ifdef __UINT32_WIDTH__
#define UINT32_WIDTH __UINT32_WIDTH__
#elif defined __INT32_WIDTH__
#define UINT32_WIDTH __INT32_WIDTH__
#else
#define UINT32_WIDTH 32
#endif
#endif

/* define INT64_WIDTH and UINT64_WIDTH if possible */
#ifdef __int64_t_defined
#ifdef __INT64_WIDTH__
#define INT64_WIDTH __INT64_WIDTH__
#elif defined __UINT64_WIDTH__
#define INT64_WIDTH __UINT64_WIDTH__
#else
#define INT64_WIDTH 64
#endif
#ifdef __UINT64_WIDTH__
#define UINT64_WIDTH __UINT64_WIDTH__
#elif defined __INT64_WIDTH__
#define UINT64_WIDTH __INT64_WIDTH__
#else
#define UINT64_WIDTH 64
#endif
#endif

/* define INT_LEAST8_WIDTH and UINT_LEAST8_WIDTH if possible */
#ifdef __INT_LEAST8_WIDTH__
#define INT_LEAST8_WIDTH __INT_LEAST8_WIDTH__
#elif defined __UINT_LEAST8_WIDTH__
#define INT_LEAST8_WIDTH __UINT_LEAST8_WIDTH__
#endif
#ifdef __UINT_LEAST8_WIDTH__
#define UINT_LEAST8_WIDTH __UINT_LEAST8_WIDTH__
#elif defined __INT_LEAST8_WIDTH__
#define UINT_LEAST8_WIDTH __INT_LEAST8_WIDTH__
#endif

/* define INT_LEAST16_WIDTH and UINT_LEAST16_WIDTH if possible */
#ifdef __INT_LEAST16_WIDTH__
#define INT_LEAST16_WIDTH __INT_LEAST16_WIDTH__
#elif defined __UINT_LEAST16_WIDTH__
#define INT_LEAST16_WIDTH __UINT_LEAST16_WIDTH__
#endif
#ifdef __UINT_LEAST16_WIDTH__
#define UINT_LEAST16_WIDTH __UINT_LEAST16_WIDTH__
#elif defined __INT_LEAST16_WIDTH__
#define UINT_LEAST16_WIDTH __INT_LEAST16_WIDTH__
#endif

/* define INT_LEAST32_WIDTH and UINT_LEAST32_WIDTH if possible */
#ifdef __INT_LEAST32_WIDTH__
#define INT_LEAST32_WIDTH __INT_LEAST32_WIDTH__
#elif defined __UINT_LEAST32_WIDTH__
#define INT_LEAST32_WIDTH __UINT_LEAST32_WIDTH__
#endif
#ifdef __UINT_LEAST32_WIDTH__
#define UINT_LEAST32_WIDTH __UINT_LEAST32_WIDTH__
#elif defined __INT_LEAST32_WIDTH__
#define UINT_LEAST32_WIDTH __INT_LEAST32_WIDTH__
#endif

/* define INT_LEAST64_WIDTH and UINT_LEAST64_WIDTH if possible */
#ifdef __INT_LEAST64_WIDTH__
#define INT_LEAST64_WIDTH __INT_LEAST64_WIDTH__
#elif defined __UINT_LEAST64_WIDTH__
#define INT_LEAST64_WIDTH __UINT_LEAST64_WIDTH__
#endif
#ifdef __UINT_LEAST64_WIDTH__
#define UINT_LEAST64_WIDTH __UINT_LEAST64_WIDTH__
#elif defined __INT_LEAST64_WIDTH__
#define UINT_LEAST64_WIDTH __INT_LEAST64_WIDTH__
#endif

/* define INT_FAST8_WIDTH and UINT_FAST8_WIDTH if possible */
#ifdef __INT_FAST8_WIDTH__
#define INT_FAST8_WIDTH __INT_FAST8_WIDTH__
#elif defined __UINT_FAST8_WIDTH__
#define INT_FAST8_WIDTH __UINT_FAST8_WIDTH__
#endif
#ifdef __UINT_FAST8_WIDTH__
#define UINT_FAST8_WIDTH __UINT_FAST8_WIDTH__
#elif defined __INT_FAST8_WIDTH__
#define UINT_FAST8_WIDTH __INT_FAST8_WIDTH__
#endif

/* define INT_FAST16_WIDTH and UINT_FAST16_WIDTH if possible */
#ifdef __INT_FAST16_WIDTH__
#define INT_FAST16_WIDTH __INT_FAST16_WIDTH__
#elif defined __UINT_FAST16_WIDTH__
#define INT_FAST16_WIDTH __UINT_FAST16_WIDTH__
#endif
#ifdef __UINT_FAST16_WIDTH__
#define UINT_FAST16_WIDTH __UINT_FAST16_WIDTH__
#elif defined __INT_FAST16_WIDTH__
#define UINT_FAST16_WIDTH __INT_FAST16_WIDTH__
#endif

/* define INT_FAST32_WIDTH and UINT_FAST32_WIDTH if possible */
#ifdef __INT_FAST32_WIDTH__
#define INT_FAST32_WIDTH __INT_FAST32_WIDTH__
#elif defined __UINT_FAST32_WIDTH__
#define INT_FAST32_WIDTH __UINT_FAST32_WIDTH__
#endif
#ifdef __UINT_FAST32_WIDTH__
#define UINT_FAST32_WIDTH __UINT_FAST32_WIDTH__
#elif defined __INT_FAST32_WIDTH__
#define UINT_FAST32_WIDTH __INT_FAST32_WIDTH__
#endif

/* define INT_FAST64_WIDTH and UINT_FAST64_WIDTH if possible */
#ifdef __INT_FAST64_WIDTH__
#define INT_FAST64_WIDTH __INT_FAST64_WIDTH__
#elif defined __UINT_FAST64_WIDTH__
#define INT_FAST64_WIDTH __UINT_FAST64_WIDTH__
#endif
#ifdef __UINT_FAST64_WIDTH__
#define UINT_FAST64_WIDTH __UINT_FAST64_WIDTH__
#elif defined __INT_FAST64_WIDTH__
#define UINT_FAST64_WIDTH __INT_FAST64_WIDTH__
#endif

/* define INTPTR_WIDTH and UINTPTR_WIDTH if possible */
#ifdef __INTPTR_WIDTH__
#define INTPTR_WIDTH __INTPTR_WIDTH__
#elif defined __UINTPTR_WIDTH__
#define INTPTR_WIDTH __UINTPTR_WIDTH__
#endif
#ifdef __UINTPTR_WIDTH__
#define UINTPTR_WIDTH __UINTPTR_WIDTH__
#elif defined __INTPTR_WIDTH__
#define UINTPTR_WIDTH __INTPTR_WIDTH__
#endif

/* define INTMAX_WIDTH and UINTMAX_WIDTH if possible */
#ifdef __INTMAX_WIDTH__
#define INTMAX_WIDTH __INTMAX_WIDTH__
#elif defined __UINTMAX_WIDTH__
#define INTMAX_WIDTH __UINTMAX_WIDTH__
#endif
#ifdef __UINTMAX_WIDTH__
#define UINTMAX_WIDTH __UINTMAX_WIDTH__
#elif defined __INTMAX_WIDTH__
#define UINTMAX_WIDTH __INTMAX_WIDTH__
#endif

/* define PTRDIFF_WIDTH if possible */
#ifdef __PTRDIFF_WIDTH__
#define PTRDIFF_WIDTH __PTRDIFF_WIDTH__
#endif

/* define SIG_ATOMIC_WIDTH if possible */
#ifdef __SIG_ATOMIC_WIDTH__
#define SIG_ATOMIC_WIDTH __SIG_ATOMIC_WIDTH__
#endif

/* define SIZE_WIDTH if possible */
#ifdef __SIZE_WIDTH__
#define SIZE_WIDTH __SIZE_WIDTH__
#endif

/* define WCHAR_WIDTH if possible */
#ifdef __WCHAR_WIDTH__
#define WCHAR_WIDTH __WCHAR_WIDTH__
#endif

/* define WINT_WIDTH if possible */
#ifdef __WINT_WIDTH__
#define WINT_WIDTH __WINT_WIDTH__
#endif

#endif /* __ISO_C_VISBLE >= 2023 */


  _END_STD_C

#endif /* _STDINT_H */

