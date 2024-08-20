/*
 * hl_toolchain.h -- provide toolchain-dependent defines.
 *
 * Copyright (c) 2024 Synopsys Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 */

#ifndef _HL_TOOLCHAIN_H
#define _HL_TOOLCHAIN_H

#ifndef __uncached
#if defined (__ARC64__)
  /* TODO: Uncached attribute is not implemented for ARCv3 yet.  */
  #define __uncached
#else
  #define __uncached __attribute__((uncached))
#endif
#endif /* __uncached */

#ifndef __aligned
  #define __aligned(x) __attribute__((aligned (x)))
#endif /* __aligned */

#ifndef __noinline
  #define __noinline __attribute__((noinline))
#endif /* __noinline */

#ifndef __always_inline
  #define __always_inline inline __attribute__((always_inline))
#endif /* __always_inline */

#ifndef __packed
  #define __packed __attribute__((packed))
#endif /* __packed */

#ifndef __noreturn
  #define __noreturn __attribute__((noreturn))
#endif /* __noreturn */

#ifndef __longcall
#if defined (__ARC64__)
  /* TODO: Long call attribute is not implemented for ARCv3 yet.  */
  #define __longcall
#else
  #define __longcall __attribute__((long_call))
#endif
#endif /* __longcall */

#define HL_MAX_DCACHE_LINE 256

#define ALIGN(x, y) (((x) + ((y) - 1)) & ~((y) - 1))

#endif /* !_HL_TOOLCHAIN_H  */
