/* stdint.h - integer types

   Copyright 2003, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _STDINT_H
#define _STDINT_H

/* Exact-width integer types */

#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#ifndef __uint32_t_defined
#define __uint32_t_defined
typedef unsigned long uint32_t;
#endif
typedef unsigned long long uint64_t;

/* Minimum-width integer types */

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef long int_least32_t;
typedef long long int_least64_t;

typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned long uint_least32_t;
typedef unsigned long long uint_least64_t;

/* Fastest minimum-width integer types */

typedef signed char int_fast8_t;
typedef long int_fast16_t;
typedef long int_fast32_t;
typedef long long int_fast64_t;

typedef unsigned char uint_fast8_t;
typedef unsigned long uint_fast16_t;
typedef unsigned long uint_fast32_t;
typedef unsigned long long uint_fast64_t;

/* Integer types capable of holding object pointers */

#ifndef __intptr_t_defined
#define __intptr_t_defined
typedef long intptr_t;
#endif
typedef unsigned long uintptr_t;

/* Greatest-width integer types */

typedef long long intmax_t;
typedef unsigned long long uintmax_t;

/* Limits of exact-width integer types */

#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1LL)

#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (9223372036854775807LL)

#define UINT8_MAX (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295UL)
#define UINT64_MAX (18446744073709551615ULL)

/* Limits of minimum-width integer types */

#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32768)
#define INT_LEAST32_MIN (-2147483647 - 1)
#define INT_LEAST64_MIN (-9223372036854775807LL - 1LL)

#define INT_LEAST8_MAX (127)
#define INT_LEAST16_MAX (32767)
#define INT_LEAST32_MAX (2147483647)
#define INT_LEAST64_MAX (9223372036854775807LL)

#define UINT_LEAST8_MAX (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295UL)
#define UINT_LEAST64_MAX (18446744073709551615ULL)

/* Limits of fastest minimum-width integer types */

#define INT_FAST8_MIN (-128)
#define INT_FAST16_MIN (-2147483647 - 1)
#define INT_FAST32_MIN (-2147483647 - 1)
#define INT_FAST64_MIN (-9223372036854775807LL - 1LL)

#define INT_FAST8_MAX (127)
#define INT_FAST16_MAX (2147483647)
#define INT_FAST32_MAX (2147483647)
#define INT_FAST64_MAX (9223372036854775807LL)

#define UINT_FAST8_MAX (255)
#define UINT_FAST16_MAX (4294967295UL)
#define UINT_FAST32_MAX (4294967295UL)
#define UINT_FAST64_MAX (18446744073709551615ULL)

/* Limits of integer types capable of holding object pointers */

#define INTPTR_MIN (-2147483647 - 1)
#define INTPTR_MAX (2147483647)
#define UINTPTR_MAX (4294967295UL)

/* Limits of greatest-width integer types */

#define INTMAX_MIN (-9223372036854775807LL - 1LL)
#define INTMAX_MAX (9223372036854775807LL)
#define UINTMAX_MAX (18446744073709551615ULL)

/* Limits of other integer types */

#ifndef PTRDIFF_MIN
#define PTRDIFF_MIN (-2147483647 - 1)
#define PTRDIFF_MAX (2147483647)
#endif

#ifndef SIG_ATOMIC_MIN
#define SIG_ATOMIC_MIN (-2147483647 - 1)
#endif
#ifndef SIG_ATOMIC_MAX
#define SIG_ATOMIC_MAX (2147483647)
#endif

#ifndef SIZE_MAX
#define SIZE_MAX (4294967295UL)
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
#define WINT_MAX UINT_MAX
#endif

/* Macros for minimum-width integer constant expressions */

#define INT8_C(x) x
#define INT16_C(x) x
#define INT32_C(x) x ## L
#define INT64_C(x) x ## LL

#define UINT8_C(x) x
#define UINT16_C(x) x
#define UINT32_C(x) x ## UL
#define UINT64_C(x) x ## ULL

/* Macros for greatest-width integer constant expressions */

#define INTMAX_C(x) x ## LL
#define UINTMAX_C(x) x ## ULL

#endif /* _STDINT_H */
