/*
 * Copyright (c) 2004, 2005 by
 * Ralf Corsepius, Ulm/Germany. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
 * @todo - Add fast<N>_t types.
 * @todo - Add support for wint_t types.
 */

#ifndef _STDINT_H
#define _STDINT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && (__GNUC__ >= 3 ) \
  && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ > 2 ) 
#define __EXP(x) __##x##__
#else
#define __EXP(x) x
#include <limits.h>
#endif

#if __EXP(SCHAR_MAX) == 0x7f
typedef signed char int8_t ;
typedef unsigned char uint8_t ;
#define __int8_t_defined 1
#endif

#if __int8_t_defined
typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;
#define __int_least8_t_defined 1
#endif

#if __EXP(SHRT_MAX) == 0x7fff
typedef signed short int16_t;
typedef unsigned short uint16_t;
#define __int16_t_defined 1
#elif __EXP(INT_MAX) == 0x7fff
typedef signed int int16_t;
typedef unsigned int uint16_t;
#define __int16_t_defined 1
#elif __EXP(SCHAR_MAX) == 0x7fff
typedef signed char int16_t;
typedef unsigned char uint16_t;
#define __int16_t_defined 1
#endif

#if __int16_t_defined
typedef int16_t   	int_least16_t;
typedef uint16_t 	uint_least16_t;
#define __int_least16_t_defined 1

#ifndef __int_least8_t_defined
typedef int16_t	   	int_least8_t;
typedef uint16_t  	uint_least8_t;
#define __int_least8_t_defined 1
#endif
#endif

#if __EXP(INT_MAX) == 0x7fffffffL
typedef signed int int32_t;
typedef unsigned int uint32_t;
#define __int32_t_defined 1
#elif __EXP(LONG_MAX) == 0x7fffffffL
typedef signed long int32_t;
typedef unsigned long uint32_t;
#define __int32_t_defined 1
#define __have_long32 1
#elif __EXP(SHRT_MAX) == 0x7fffffffL
typedef signed short int32_t;
typedef unsigned short uint32_t;
#define __int32_t_defined 1
#elif __EXP(SCHAR_MAX) == 0x7fffffffL
typedef signed char int32_t;
typedef unsigned char uint32_t;
#define __int32_t_defined 1
#endif

#if __int32_t_defined
typedef int32_t   	int_least32_t;
typedef uint32_t 	uint_least32_t;
#define __int_least32_t_defined 1

#ifndef __int_least8_t_defined
typedef int32_t	   	int_least8_t;
typedef uint32_t  	uint_least8_t;
#define __int_least8_t_defined 1
#endif

#ifndef __int_least16_t_defined
typedef int32_t	   	int_least16_t;
typedef uint32_t  	uint_least16_t;
#define __int_least16_t_defined 1
#endif
#endif

#if __EXP(LONG_MAX) > 0x7fffffff
typedef signed long int64_t;
typedef unsigned long uint64_t;
#define __int64_t_defined 1
#define __have_long64 1
#elif  defined(__LONG_LONG_MAX__) && (__LONG_LONG_MAX__ > 0x7fffffff)
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#define __int64_t_defined 1
#define __have_longlong64 1
#elif  defined(LLONG_MAX) && (LLONG_MAX > 0x7fffffff)
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#define __int64_t_defined 1
#define __have_longlong64 1
#elif  __EXP(INT_MAX) > 0x7fffffff
typedef signed int int64_t;
typedef unsigned int uint64_t;
#define __int64_t_defined 1
#endif

#if __int64_t_defined
typedef int64_t   	int_least64_t;
typedef uint64_t 	uint_least64_t;
#define __int_least64_t_defined 1

#ifndef __int_least8_t_defined
typedef int64_t	   	int_least8_t;
typedef uint64_t  	uint_least8_t;
#define __int_least8_t_defined 1
#endif

#ifndef __int_least16_t_defined
typedef int64_t	   	int_least16_t;
typedef uint64_t  	uint_least16_t;
#define __int_least16_t_defined 1
#endif

#ifndef __int_least32_t_defined
typedef int64_t	   	int_least32_t;
typedef uint64_t  	uint_least32_t;
#define __int_least32_t_defined 1
#endif
#endif

#if __have_longlong64
typedef signed long long intmax_t;
typedef unsigned long long uintmax_t;
#else
typedef signed long intmax_t;
typedef unsigned long uintmax_t;
#endif

/*
 * GCC doesn't provide an propriate macro for [u]intptr_t
 * For now, use __PTRDIFF_TYPE__
 */
#if defined(__PTRDIFF_TYPE__)
typedef signed __PTRDIFF_TYPE__ intptr_t;
typedef unsigned __PTRDIFF_TYPE__ uintptr_t;
#else
/*
 * Fallback to hardcoded values, 
 * should be valid on cpu's with 32bit int/32bit void*
 */
typedef signed long intptr_t;
typedef unsigned long uintptr_t;
#endif

/* Limits of Specified-Width Integer Types */

#if __int8_t_defined
#define INT8_MIN 	-128
#define INT8_MAX 	 127
#define UINT8_MAX 	 255
#endif

#if __int_least8_t_defined
#define INTLEAST8_MIN 	-128
#define INTLEAST8_MAX 	 127
#define UINTLEAST8_MAX 	 255
#else
#error required type int_least8_t missing
#endif

#if __int16_t_defined
#define INT16_MIN 	-32768
#define INT16_MAX 	 32767
#define UINT16_MAX 	 65535
#endif

#if __int_least16_t_defined
#define INTLEAST16_MIN 	-32768
#define INTLEAST16_MAX 	 32767
#define UINTLEAST16_MAX  65535
#else
#error required type int_least16_t missing
#endif

#if __int32_t_defined
#define INT32_MIN 	-2147483648
#define INT32_MAX 	 2147483647
#define UINT32_MAX       4294967295
#endif

#if __int_least32_t_defined
#define INTLEAST32_MIN 	-2147483648
#define INTLEAST32_MAX 	 2147483647
#define UINTLEAST32_MAX  4294967295
#else
#error required type int_least32_t missing
#endif

#if __int64_t_defined
#define INT64_MIN 	-9223372036854775808
#define INT64_MAX 	 9223372036854775807
#define UINT64_MAX 	18446744073709551615
#endif

#if __int_least64_t_defined
#define INTLEAST64_MIN 	-9223372036854775808
#define INTLEAST64_MAX 	 9223372036854775807
#define UINTLEAST64_MAX 18446744073709551615
#endif

/* This must match size_t in stddef.h, currently long unsigned int */
#define SIZE_MIN (-__EXP(LONG_MAX) - 1L)
#define SIZE_MAX __EXP(LONG_MAX)

/* This must match sig_atomic_t in <signal.h> (currently int) */
#define SIG_ATOMIC_MIN (-__EXP(INT_MAX) - 1)
#define SIG_ATOMIC_MAX __EXP(INT_MAX)

/* This must match ptrdiff_t  in <stddef.h> (currently long int) */
#define PTRDIFF_MIN (-__EXP(LONG_MAX) - 1L)
#define PTHDIFF_MAX __EXT(LONG_MAX)

#undef __EXP

/** Macros for minimum-width integer constant expressions */
#define INT8_C(x)	x
#define UINT8_C(x)	x##U

#define INT16_C(x)	x
#define UINT16_C(x)	x##U

#if __have_long32
#define INT32_C(x)	x##L
#define UINT32_C(x)	x##UL
#else
#define INT32_C(x)	x
#define UINT32_C(x)	x##U
#endif

#if __int64_t_defined
#if __have_longlong64
#define INT64_C(x)	x##LL
#define UINT64_C(x)	x##ULL
#else
#define INT64_C(x)	x##L
#define UINT64_C(x)	x##UL
#endif
#endif

/** Macros for greatest-width integer constant expression */
#if __have_longlong64
#define INTMAX_C(x)	x##LL
#define UINTMAX_C(x)	x##ULL
#else
#define INTMAX_C(x)	x##L
#define UINTMAX_C(x)	x##UL
#endif


#ifdef __cplusplus
}
#endif

#endif /* _STDINT_H */
