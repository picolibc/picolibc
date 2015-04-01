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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ___int8_t_defined
typedef __int8_t int8_t ;
typedef __uint8_t uint8_t ;
#define __int8_t_defined 1
#endif

#ifdef ___int16_t_defined
typedef __int16_t int16_t ;
typedef __uint16_t uint16_t ;
#define __int16_t_defined 1
#endif

#ifdef ___int32_t_defined
typedef __int32_t int32_t ;
typedef __uint32_t uint32_t ;
#define __int32_t_defined 1
#endif

#ifdef ___int64_t_defined
typedef __int64_t int64_t ;
typedef __uint64_t uint64_t ;
#define __int64_t_defined 1
#endif

typedef __intptr_t intptr_t;
typedef __uintptr_t uintptr_t;

#ifdef __cplusplus
}
#endif

#endif /* _SYS__STDINT_H */
