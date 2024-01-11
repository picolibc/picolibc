/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MACHINE_MATH_H_
#define _MACHINE_MATH_H_

#if defined(__riscv_flen) || defined(__riscv_zfinx)

#if (__riscv_flen >= 64) || defined(__riscv_zdinx)
#define __RISCV_HARD_FLOAT 64
#else
#define __RISCV_HARD_FLOAT 32
#endif

#ifdef _WANT_MATH_ERRNO
#include <errno.h>
#endif

#define FCLASS_NEG_INF       (1 << 0)
#define FCLASS_NEG_NORMAL    (1 << 1)
#define FCLASS_NEG_SUBNORMAL (1 << 2)
#define FCLASS_NEG_ZERO      (1 << 3)
#define FCLASS_POS_ZERO      (1 << 4)
#define FCLASS_POS_SUBNORMAL (1 << 5)
#define FCLASS_POS_NORMAL    (1 << 6)
#define FCLASS_POS_INF       (1 << 7)
#define FCLASS_SNAN          (1 << 8)
#define FCLASS_QNAN          (1 << 9)

#define FCLASS_INF           (FCLASS_NEG_INF | FCLASS_POS_INF)
#define FCLASS_ZERO          (FCLASS_NEG_ZERO | FCLASS_POS_ZERO)
#define FCLASS_NORMAL        (FCLASS_NEG_NORMAL | FCLASS_POS_NORMAL)
#define FCLASS_SUBNORMAL     (FCLASS_NEG_SUBNORMAL | FCLASS_POS_SUBNORMAL)
#define FCLASS_NAN           (FCLASS_SNAN | FCLASS_QNAN)

#if __RISCV_HARD_FLOAT >= 64

/* anything with a 64-bit FPU has FMA */
#define _HAVE_FAST_FMA 1

#define _fclass_d(_x) (__extension__(                                   \
                               {                                        \
                                       long __fclass;                   \
                                       __asm__("fclass.d %0, %1" :      \
                                               "=r" (__fclass) :        \
                                               "f" (_x));               \
                                       __fclass;                        \
                               }))

#endif

#if __RISCV_HARD_FLOAT >= 32

/* anything with a 32-bit FPU has FMAF */
#define _HAVE_FAST_FMAF 1

#define _fclass_f(_x) (__extension__(                                   \
                               {                                        \
                                       long __fclass;                   \
                                       __asm__("fclass.s %0, %1" :      \
                                               "=r" (__fclass) :        \
                                               "f" (_x));               \
                                       __fclass;                        \
                               }))

#endif


#ifdef __declare_extern_inline

#if __RISCV_HARD_FLOAT >= 64

/* Double-precision functions */
__declare_extern_inline(double)
copysign(double x, double y)
{
	double result;
	__asm__("fsgnj.d\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_extern_inline(double)
fabs(double x)
{
	double result;
	__asm__("fabs.d\t%0, %1" : "=f"(result) : "f"(x));
	return result;
}

__declare_extern_inline(double)
fmax (double x, double y)
{
	double result;
        if (issignaling(x) || issignaling(y))
            return x + y;

	__asm__ volatile("fmax.d\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_extern_inline(double)
fmin (double x, double y)
{
	double result;
        if (issignaling(x) || issignaling(y))
            return x + y;

	__asm__ volatile("fmin.d\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_extern_inline(int)
__finite(double x)
{
	long fclass = _fclass_d (x);
	return (fclass & (FCLASS_INF|FCLASS_NAN)) == 0;
}

__declare_extern_inline(int)
finite(double x)
{
        return __finite(x);
}

__declare_extern_inline(int)
__fpclassifyd (double x)
{
  long fclass = _fclass_d (x);

  if (fclass & FCLASS_ZERO)
    return FP_ZERO;
  else if (fclass & FCLASS_NORMAL)
    return FP_NORMAL;
  else if (fclass & FCLASS_SUBNORMAL)
    return FP_SUBNORMAL;
  else if (fclass & FCLASS_INF)
    return FP_INFINITE;
  else
    return FP_NAN;
}

__declare_extern_inline(double)
sqrt (double x)
{
	double result;
#ifdef _WANT_MATH_ERRNO
        if (isless(x, 0.0))
            errno = EDOM;
#endif
	__asm__ volatile("fsqrt.d %0, %1" : "=f" (result) : "f" (x));
	return result;
}

__declare_extern_inline(double)
fma (double x, double y, double z)
{
	double result;
	__asm__ volatile("fmadd.d %0, %1, %2, %3" : "=f" (result) : "f" (x), "f" (y), "f" (z));
	return result;
}

#endif /* __RISCV_HARD_FLOAT >= 64 */

#if __RISCV_HARD_FLOAT >= 32

/* Single-precision functions */
__declare_extern_inline(float)
copysignf(float x, float y)
{
	float result;
	__asm__("fsgnj.s\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_extern_inline(float)
fabsf (float x)
{
	float result;
	__asm__("fabs.s\t%0, %1" : "=f"(result) : "f"(x));
	return result;
}

__declare_extern_inline(float)
fmaxf (float x, float y)
{
	float result;
        if (issignaling(x) || issignaling(y))
            return x + y;

	__asm__ volatile("fmax.s\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_extern_inline(float)
fminf (float x, float y)
{
	float result;
        if (issignaling(x) || issignaling(y))
            return x + y;

	__asm__ volatile("fmin.s\t%0, %1, %2" : "=f" (result) : "f" (x), "f" (y));
	return result;
}

__declare_extern_inline(int)
__finitef(float x)
{
	long fclass = _fclass_f (x);
	return (fclass & (FCLASS_INF|FCLASS_NAN)) == 0;
}

__declare_extern_inline(int)
finitef(float x)
{
        return __finitef(x);
}

__declare_extern_inline(int)
__fpclassifyf (float x)
{
  long fclass = _fclass_f (x);

  if (fclass & FCLASS_ZERO)
    return FP_ZERO;
  else if (fclass & FCLASS_NORMAL)
    return FP_NORMAL;
  else if (fclass & FCLASS_SUBNORMAL)
    return FP_SUBNORMAL;
  else if (fclass & FCLASS_INF)
    return FP_INFINITE;
  else
    return FP_NAN;
}

__declare_extern_inline(float)
sqrtf (float x)
{
	float result;
#ifdef _WANT_MATH_ERRNO
        if (isless(x, 0.0f))
            errno = EDOM;
#endif
	__asm__ volatile("fsqrt.s %0, %1" : "=f" (result) : "f" (x));
	return result;
}

__declare_extern_inline(float)
fmaf (float x, float y, float z)
{
	float result;
	__asm__ volatile("fmadd.s %0, %1, %2, %3" : "=f" (result) : "f" (x), "f" (y), "f" (z));
	return result;
}

#endif /* __RISCV_HARD_FLOAT >= 32 */

#endif /* defined(__GNUC_GNU_INLINE__) || defined(__GNUC_STDC_INLINE__) */

#endif /* __RISCV_HARD_FLOAT */

#endif /* _MACHINE_MATH_H_ */
