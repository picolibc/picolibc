/*
 * Copyright (c) 2004, 2005 by
 * Ralf Corsepius, Ulm/Germany. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#ifndef _SYS__INTSUP_H
#define _SYS__INTSUP_H

#include <sys/features.h>

#if __GNUC_PREREQ (3, 2)
/* gcc > 3.2 implicitly defines the values we are interested */
#define __STDINT_EXP(x) __##x##__
#else
#define __STDINT_EXP(x) x
#include <limits.h>
#endif

/* Check if "long long" is 64bit wide */
/* Modern GCCs provide __LONG_LONG_MAX__, SUSv3 wants LLONG_MAX */
#if ( defined(__LONG_LONG_MAX__) && (__LONG_LONG_MAX__ > 0x7fffffff) ) \
  || ( defined(LLONG_MAX) && (LLONG_MAX > 0x7fffffff) )
#define __have_longlong64 1
#endif

/* Check if "long" is 64bit or 32bit wide */
#if __STDINT_EXP(LONG_MAX) > 0x7fffffff
#define __have_long64 1
#elif __STDINT_EXP(LONG_MAX) == 0x7fffffff && !defined(__SPU__)
#define __have_long32 1
#endif

/* Determine how intptr_t and int32_t are defined by gcc for this target. This
   is used to determine the correct printf() constant in inttypes.h and other
   constants in stdint.h. */
#pragma push_macro("signed")
#pragma push_macro("int")
#pragma push_macro("long")
#undef signed
#undef int
#undef long
#define signed +0
#define int +0
#define long +1
#if __INTPTR_TYPE__ == 2
#define _INTPTR_EQ_LONGLONG
#elif __INTPTR_TYPE__ == 1
#define _INTPTR_EQ_LONG
#elif __INTPTR_TYPE__ == 0
/* Nothing to define because intptr_t is safe to print as an int. */
#else
#error "Unable to determine type definition of intptr_t"
#endif
#if __INT32_TYPE__ == 1
#define _INT32_EQ_LONG
#elif __INT32_TYPE__ == 0
/* Nothing to define because int32_t is safe to print as an int. */
#else
#error "Unable to determine type definition of int32_t"
#endif
#undef long
#undef int
#undef signed
#pragma pop_macro("signed")
#pragma pop_macro("int")
#pragma pop_macro("long") 

#endif /* _SYS__INTSUP_H */
