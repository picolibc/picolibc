/*
 *  Copyright (c) 2007 Patrick Mansfield <patmans@us.ibm.com>
 */

#ifndef _MACHINE__DEFAULT_TYPES_H
#define _MACHINE__DEFAULT_TYPES_H

#include <sys/features.h>

/*
 * Guess on types by examining *_MIN / *_MAX defines.
 */
#if defined(__INT_MAX__) && defined(__LONG_MAX__) && defined (__SCHAR_MAX__) && defined (__SHRT_MAX__)
/* GCC >= 3.3.0 has __<val>__ implicitly defined. */
#define __EXP(x) __##x##__
#else
/* Fall back to POSIX versions from <limits.h> */
#define __EXP(x) x
#include <limits.h>
#endif

/* Check if "long long" is 64bit wide */
/* Modern GCCs provide __LONG_LONG_MAX__, SUSv3 wants LLONG_MAX */
#if ( defined(__LONG_LONG_MAX__) && (__LONG_LONG_MAX__ > 0x7fffffff) ) \
  || ( defined(LLONG_MAX) && (LLONG_MAX > 0x7fffffff) )
#define __have_longlong64 1
#endif

/* Check if "long" is 64bit or 32bit wide */
#if __EXP(LONG_MAX) > 0x7fffffff
#define __have_long64 1
#elif __EXP(LONG_MAX) == 0x7fffffff && !defined(__SPU__)
#define __have_long32 1
#endif

/* Determine the width of integers if the compiler doesn't provide __X_WIDTH__ macros. */

#if !defined(__SCHAR_WIDTH__)
#if __EXP(SCHAR_MAX) == 0x7f
#define __SCHAR_WIDTH__ 8
#elif __EXP(SCHAR_MAX) == 0x7fff
#define __SCHAR_WIDTH__ 16
#elif __EXP(SCHAR_MAX) == 0x7fffffffL
#define __SCHAR_WIDTH__ 32
#endif
#endif

#if !defined(__SHRT_WIDTH__)
#if __EXP(SHRT_MAX) == 0x7fff
#define __SHRT_WIDTH__ 16
#elif __EXP(SHRT_MAX) == 0x7fffffffL
#define __SHRT_WIDTH__ 32
#endif
#endif

#if !defined(__INT_WIDTH__)
#if __EXP(INT_MAX) == 0x7fff
#define __INT_WIDTH__ 16
#elif __EXP(INT_MAX) == 0x7fffffffL
#define __INT_WIDTH__ 32
#elif __EXP(INT_MAX) > 0x7fffffffL
#define __INT_WIDTH__ 64
#endif
#endif

#if !defined(__LONG_WIDTH__)
#if __have_long32
#define __LONG_WIDTH__ 32
#elif __have_long64
#define __LONG_WIDTH__ 64
#endif
#endif

#if !defined(__LONG_LONG_WIDTH__)
#if __have_longlong64
#define __LONG_LONG_WIDTH__ 64
#endif
#endif

/* Determine the size of types if the compiler doesn't provide __SIZEOF_X__ macros. */

#if !defined(__SIZEOF_SHORT__)
#if __SHRT_WIDTH__ == 16
#define __SIZEOF_SHORT__ 2
#elif __SHRT_WIDTH__ == 32
#define __SIZEOF_SHORT__ 4
#endif
#endif

#if !defined(__SIZEOF_INT__)
#if __INT_WIDTH__ == 16
#define __SIZEOF_INT__ 2
#elif __INT_WIDTH__ == 32
#define __SIZEOF_INT__ 4
#elif __INT_WIDTH__ == 64
#define __SIZEOF_INT__ 8
#endif
#endif

#ifndef __SIZEOF_LONG__
#if __LONG_WIDTH__ == 32
#define __SIZEOF_LONG__ 4
#elif __LONG_WIDTH__ == 64
#define __SIZEOF_LONG__ 8
#endif
#endif

#if (!defined(__SIZEOF_LONG_LONG__) && __LONG_LONG_WIDTH__ == 64)
#define __SIZEOF_LONG_LONG__ 8
#endif

#if (!defined(__SIZEOF_FLOAT__) && defined(__FLT_MANT_DIG__))
#if __FLT_MANT_DIG__ == 24
#define __SIZEOF_FLOAT__ 4
#else
#error "unexpected __FLT_MANT_DIG__ value"
#endif
#endif

#if (!defined(__SIZEOF_DOUBLE__) && defined(__DBL_MANT_DIG__))
#if __DBL_MANT_DIG__ == 53
#define __SIZEOF_DOUBLE__ 8
#else
#error "unexpected __DBL_MANT_DIG__ value"
#endif
#endif

#if (!defined(__HAVE_LONG_DOUBLE) && defined(__LDBL_MANT_DIG__))
#if __LDBL_MANT_DIG__ == 53
#define __SIZEOF_LONG_DOUBLE__ 8
#elif __LDBL_MANT_DIG__ == 64 || __LDBL_MANT_DIG__ == 113
#define __SIZEOF_LONG_DOUBLE__ 16
#else
#error "unexpected __LDBL_MANT_DIG__ values"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Select type of fixed width integers if the compiler doesn't specify them. */

#if !defined(__INT8_TYPE__) && __SCHAR_WIDTH__ == 8
#define __INT8_TYPE__ signed char
#endif

#if !defined(__UINT8_TYPE__) && __SCHAR_WIDTH__ == 8
#define __UINT8_TYPE__ unsigned char
#endif

#if !defined(__INT16_TYPE__)
#if __INT_WIDTH__ == 16
#define __INT16_TYPE__ int
#elif __SHRT_WIDTH__ == 16
#define __INT16_TYPE__ short
#elif __SCHAR_WIDTH__ == 16
#define __INT16_TYPE__ signed char
#if !defined(__UINT16_TYPE__)
#define __UINT16_TYPE__ unsigned char
#endif
#endif
#endif

#if !defined(__UINT16_TYPE__) && defined(__INT16_TYPE__)
#define __UINT16_TYPE__ unsigned __INT16_TYPE__
#endif

#if !defined(__INT32_TYPE__)
#if __INT_WIDTH__ == 32
#define __INT32_TYPE__ int
#elif __LONG_WIDTH__ == 32
#define __INT32_TYPE__ long
#elif __SHRT_WIDTH__ == 32
#define __INT32_TYPE__ short
#elif __SCHAR_WIDTH__ == 32
#define __INT32_TYPE__ signed char
#if !defined(__UINT32_TYPE__)
#define __UINT32_TYPE__ unsigned char
#endif
#endif
#endif

#if !defined(__UINT32_TYPE__) && defined(__INT32_TYPE__)
#define __UINT32_TYPE__ unsigned __INT32_TYPE__
#endif

#if !defined(__INT64_TYPE__)
#if __LONG_WIDTH__ == 64
#define __INT64_TYPE__ long
#elif __LONG_LONG_WIDTH__ == 64
#define __INT64_TYPE__ long long
#endif
#endif

#if !defined(__UINT64_TYPE__) && defined(__INT64_TYPE__)
#define __UINT64_TYPE__ unsigned __INT64_TYPE__
#endif

#if defined(__GNUC__) && !__GNUC_PREREQ (4, 5)
#ifndef __INTPTR_TYPE__
#define __INTPTR_TYPE__ long int
#endif
#ifndef __UINTPTR_TYPE__
#define __UINTPTR_TYPE__ long unsigned int
#endif
#endif

#ifdef __INT8_TYPE__
typedef __INT8_TYPE__ __int8_t;
#define ___int8_t_defined 1
#endif

#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ __uint8_t;
#endif

#ifdef __INT16_TYPE__
typedef __INT16_TYPE__ __int16_t;
#define ___int16_t_defined 1
#endif

#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ __uint16_t;
#endif

#ifdef __INT32_TYPE__
typedef __INT32_TYPE__ __int32_t;
#define ___int32_t_defined 1
#endif

#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ __uint32_t;
#endif

#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ __int64_t;
#define ___int64_t_defined 1
#endif

#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ __uint64_t;
#endif

#ifdef __INT_LEAST8_TYPE__
typedef __INT_LEAST8_TYPE__ __int_least8_t;
#ifdef __UINT_LEAST8_TYPE__
typedef __UINT_LEAST8_TYPE__ __uint_least8_t;
#else
typedef unsigned __INT_LEAST8_TYPE__ __uint_least8_t;
#endif
#define ___int_least8_t_defined 1
#elif defined(___int8_t_defined)
typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
#define ___int_least8_t_defined 1
#elif defined(___int16_t_defined)
typedef __int16_t __int_least8_t;
typedef __uint16_t __uint_least8_t;
#define ___int_least8_t_defined 1
#elif defined(___int32_t_defined)
typedef __int32_t __int_least8_t;
typedef __uint32_t __uint_least8_t;
#define ___int_least8_t_defined 1
#elif defined(___int64_t_defined)
typedef __int64_t __int_least8_t;
typedef __uint64_t __uint_least8_t;
#define ___int_least8_t_defined 1
#endif

#ifdef __INT_LEAST16_TYPE__
typedef __INT_LEAST16_TYPE__ __int_least16_t;
#ifdef __UINT_LEAST16_TYPE__
typedef __UINT_LEAST16_TYPE__ __uint_least16_t;
#else
typedef unsigned __INT_LEAST16_TYPE__ __uint_least16_t;
#endif
#define ___int_least16_t_defined 1
#elif defined(___int16_t_defined)
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
#define ___int_least16_t_defined 1
#elif defined(___int32_t_defined)
typedef __int32_t __int_least16_t;
typedef __uint32_t __uint_least16_t;
#define ___int_least16_t_defined 1
#elif defined(___int64_t_defined)
typedef __int64_t __int_least16_t;
typedef __uint64_t __uint_least16_t;
#define ___int_least16_t_defined 1
#endif

#ifdef __INT_LEAST32_TYPE__
typedef __INT_LEAST32_TYPE__ __int_least32_t;
#ifdef __UINT_LEAST32_TYPE__
typedef __UINT_LEAST32_TYPE__ __uint_least32_t;
#else
typedef unsigned __INT_LEAST32_TYPE__ __uint_least32_t;
#endif
#define ___int_least32_t_defined 1
#elif defined(___int32_t_defined)
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
#define ___int_least32_t_defined 1
#elif defined(___int64_t_defined)
typedef __int64_t __int_least32_t;
typedef __uint64_t __uint_least32_t;
#define ___int_least32_t_defined 1
#endif

#ifdef __INT_LEAST64_TYPE__
typedef __INT_LEAST64_TYPE__ __int_least64_t;
#ifdef __UINT_LEAST64_TYPE__
typedef __UINT_LEAST64_TYPE__ __uint_least64_t;
#else
typedef unsigned __INT_LEAST64_TYPE__ __uint_least64_t;
#endif
#define ___int_least64_t_defined 1
#elif defined(___int64_t_defined)
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;
#define ___int_least64_t_defined 1
#endif

#if defined(__INTMAX_TYPE__)
typedef __INTMAX_TYPE__ __intmax_t;
#elif __have_longlong64
typedef signed long long __intmax_t;
#else
typedef signed long __intmax_t;
#endif

#if defined(__UINTMAX_TYPE__)
typedef __UINTMAX_TYPE__ __uintmax_t;
#elif __have_longlong64
typedef unsigned long long __uintmax_t;
#else
typedef unsigned long __uintmax_t;
#endif

#ifdef __INTPTR_TYPE__
typedef __INTPTR_TYPE__ __intptr_t;
#ifdef __UINTPTR_TYPE__
typedef __UINTPTR_TYPE__ __uintptr_t;
#else
typedef unsigned __INTPTR_TYPE__ __uintptr_t;
#endif
#elif defined(__PTRDIFF_TYPE__)
typedef __PTRDIFF_TYPE__ __intptr_t;
typedef unsigned __PTRDIFF_TYPE__ __uintptr_t;
#else
typedef long __intptr_t;
typedef unsigned long __uintptr_t;
#endif

#undef __EXP

#ifdef __cplusplus
}
#endif

#endif /* _MACHINE__DEFAULT_TYPES_H */
