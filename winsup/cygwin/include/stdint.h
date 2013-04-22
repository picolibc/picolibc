/* stdint.h - integer types

   Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _STDINT_H
#define _STDINT_H

#include <bits/wordsize.h>

/* Exact-width integer types */

#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
#if __WORDSIZE == 64
typedef long int64_t;
#else
typedef long long int64_t;
#endif
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#ifndef __uint32_t_defined
#define __uint32_t_defined
typedef unsigned int uint32_t;
#endif
#if __WORDSIZE == 64
typedef unsigned long uint64_t;
#else
typedef unsigned long long uint64_t;
#endif

/* Minimum-width integer types */

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef int int_least32_t;
#if __WORDSIZE == 64
typedef long int_least64_t;
#else
typedef long long int_least64_t;
#endif

typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned int uint_least32_t;
#if __WORDSIZE == 64
typedef unsigned long uint_least64_t;
#else
typedef unsigned long long uint_least64_t;
#endif

/* Fastest minimum-width integer types */

typedef signed char int_fast8_t;
#if __WORDSIZE == 64
typedef long int_fast16_t;
typedef long int_fast32_t;
typedef long int_fast64_t;
#else
typedef int int_fast16_t;
typedef int int_fast32_t;
typedef long long int_fast64_t;
#endif

typedef unsigned char uint_fast8_t;
#if __WORDSIZE == 64
typedef unsigned long uint_fast16_t;
typedef unsigned long uint_fast32_t;
typedef unsigned long uint_fast64_t;
#else
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
typedef unsigned long long uint_fast64_t;
#endif

/* Integer types capable of holding object pointers */

#ifndef __intptr_t_defined
#define __intptr_t_defined
#if __WORDSIZE == 64
typedef long intptr_t;
#else
typedef int intptr_t;
#endif
#endif
#if __WORDSIZE == 64
typedef unsigned long uintptr_t;
#else
typedef unsigned int uintptr_t;
#endif

/* Greatest-width integer types */

#if __WORDSIZE == 64
typedef long intmax_t;
typedef unsigned long uintmax_t;
#else
typedef long long intmax_t;
typedef unsigned long long uintmax_t;
#endif

/* C99 requires that in C++ the following macros should be defined only
   if requested. */
#if !defined (__cplusplus) || defined (__STDC_LIMIT_MACROS) \
    || defined (__INSIDE_CYGWIN__)

#if __x86_64__
# define __I64(n) n ## L
# define __U64(n) n ## UL
#else
# define __I64(n) n ## LL
# define __U64(n) n ## ULL
#endif

/* Limits of exact-width integer types */

#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-__I64(9223372036854775807) - 1)

#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (__I64(9223372036854775807))

#define UINT8_MAX (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (__U64(18446744073709551615))

/* Limits of minimum-width integer types */

#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32768)
#define INT_LEAST32_MIN (-2147483647 - 1)
#define INT_LEAST64_MIN (-__I64(9223372036854775807) - 1)

#define INT_LEAST8_MAX (127)
#define INT_LEAST16_MAX (32767)
#define INT_LEAST32_MAX (2147483647)
#define INT_LEAST64_MAX (__I64(9223372036854775807))

#define UINT_LEAST8_MAX (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295U)
#define UINT_LEAST64_MAX (__U64(18446744073709551615))

/* Limits of fastest minimum-width integer types */

#define INT_FAST8_MIN (-128)
#if __WORDSIZE == 64
#define INT_FAST16_MIN (-__I64(9223372036854775807) - 1)
#define INT_FAST32_MIN (-__I64(9223372036854775807) - 1)
#else
#define INT_FAST16_MIN (-2147483647 - 1)
#define INT_FAST32_MIN (-2147483647 - 1)
#endif
#define INT_FAST64_MIN (-__I64(9223372036854775807) - 1)

#define INT_FAST8_MAX (127)
#if __WORDSIZE == 64
#define INT_FAST16_MAX (__I64(9223372036854775807))
#define INT_FAST32_MAX (__I64(9223372036854775807))
#else
#define INT_FAST16_MAX (2147483647)
#define INT_FAST32_MAX (2147483647)
#endif
#define INT_FAST64_MAX (__I64(9223372036854775807))

#define UINT_FAST8_MAX (255)
#if __WORDSIZE == 64
#define UINT_FAST16_MAX (__U64(18446744073709551615))
#define UINT_FAST32_MAX (__U64(18446744073709551615))
#else
#define UINT_FAST16_MAX (4294967295U)
#define UINT_FAST32_MAX (4294967295U)
#endif
#define UINT_FAST64_MAX (__U64(18446744073709551615))

/* Limits of integer types capable of holding object pointers */

#if __WORDSIZE == 64
#define INTPTR_MIN (-__I64(9223372036854775807) - 1)
#define INTPTR_MAX (__I64(9223372036854775807))
#define UINTPTR_MAX (__U64(18446744073709551615))
#else
#define INTPTR_MIN (-2147483647 - 1)
#define INTPTR_MAX (2147483647)
#define UINTPTR_MAX (4294967295U)
#endif

/* Limits of greatest-width integer types */

#define INTMAX_MIN (-__I64(9223372036854775807) - 1)
#define INTMAX_MAX (__I64(9223372036854775807))
#define UINTMAX_MAX (__U64(18446744073709551615))

/* Limits of other integer types */

#ifndef PTRDIFF_MIN
#if __WORDSIZE == 64
#define PTRDIFF_MIN (-9223372036854775807L - 1)
#define PTRDIFF_MAX (9223372036854775807L)
#else
#define PTRDIFF_MIN (-2147483647 - 1)
#define PTRDIFF_MAX (2147483647)
#endif
#endif

#ifndef SIG_ATOMIC_MIN
#define SIG_ATOMIC_MIN (-2147483647 - 1)
#endif
#ifndef SIG_ATOMIC_MAX
#define SIG_ATOMIC_MAX (2147483647)
#endif

#ifndef SIZE_MAX
#if __WORDSIZE == 64
#define SIZE_MAX (18446744073709551615UL)
#else
#define SIZE_MAX (4294967295U)
#endif
#endif

#ifndef WCHAR_MIN
#ifdef __WCHAR_MIN__
#define WCHAR_MIN __WCHAR_MIN__
#define WCHAR_MAX __WCHAR_MAX__
#else
#define WCHAR_MIN (0)
#define WCHAR_MAX (65535)
#endif
#endif

#ifndef WINT_MIN
#define WINT_MIN 0U
#define WINT_MAX (4294967295U)
#endif

#endif /* !__cplusplus || __STDC_LIMIT_MACROS || __INSIDE_CYGWIN__ */

/* C99 requires that in C++ the following macros should be defined only
   if requested. */
#if !defined (__cplusplus) || defined (__STDC_CONSTANT_MACROS) \
    || defined (__INSIDE_CYGWIN__)

/* Macros for minimum-width integer constant expressions */

#define INT8_C(x) x
#define INT16_C(x) x
#define INT32_C(x) x
#if __WORDSIZE == 64
#define INT64_C(x) x ## L
#else
#define INT64_C(x) x ## LL
#endif

#define UINT8_C(x) x
#define UINT16_C(x) x
#define UINT32_C(x) x ## U
#if __WORDSIZE == 64
#define UINT64_C(x) x ## UL
#else
#define UINT64_C(x) x ## ULL
#endif

/* Macros for greatest-width integer constant expressions */

#if __WORDSIZE == 64
#define INTMAX_C(x) x ## L
#define UINTMAX_C(x) x ## UL
#else
#define INTMAX_C(x) x ## LL
#define UINTMAX_C(x) x ## ULL
#endif

#endif /* !__cplusplus || __STDC_CONSTANT_MACROS || __INSIDE_CYGWIN__ */

#endif /* _STDINT_H */
