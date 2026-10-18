/*
 * Copyright (c) 2004, 2005 by
 * Ralf Corsepius, Ulm/Germany. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#ifndef _SYS__STDINT_H
#define _SYS__STDINT_H

#include <machine/_default_types.h>

_BEGIN_STD_C

#include <sys/_exactint.h>

#ifndef _INTMAX_T_DECLARED
typedef __intmax_t intmax_t;
#define _INTMAX_T_DECLARED
#endif

#ifndef _UINTMAX_T_DECLARED
typedef __uintmax_t uintmax_t;
#define _UINTMAX_T_DECLARED
#endif

#ifndef _INTPTR_T_DECLARED
typedef __intptr_t intptr_t;
#define _INTPTR_T_DECLARED
#endif

#ifndef _UINTPTR_T_DECLARED
typedef __uintptr_t uintptr_t;
#define _UINTPTR_T_DECLARED
#endif

_END_STD_C

#endif /* _SYS__STDINT_H */
