/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#define __HAVE_FAST_FMA  1
#define __HAVE_FAST_FMAF 1

#ifdef __declare_extern_inline

#ifdef __MATH_ERRNO
#include <errno.h>
#endif

__declare_extern_inline(double)
fma(double x, double y, double z)
{
    extern double __hexagon_fmadf5(double, double, double);
    return __hexagon_fmadf5(x, y, z);
}

__declare_extern_inline(double)
fmax(double x, double y)
{
    double result;
    if (issignaling(x) || issignaling(y))
        return x + y;

    __asm__("%0 = dfmax(%1, %2)" : "=r"(result) : "r"(x), "r"(y));
    return result;
}

__declare_extern_inline(double)
fmin(double x, double y)
{
    double result;
    if (issignaling(x) || issignaling(y))
        return x + y;

    __asm__("%0 = dfmin(%1, %2)" : "=r"(result) : "r"(x), "r"(y));
    return result;
}

__declare_extern_inline(double)
sqrt(double x)
{
    extern double __hexagon_sqrt(double);
#ifdef __MATH_ERRNO
    if (isless(x, 0.0))
        errno = EDOM;
#endif
    return __hexagon_sqrt(x);
}

__declare_extern_inline(float)
fmaf(float x, float y, float z)
{
    float result = z;
    __asm__("%0 += sfmpy(%1, %2)" : "+r"(result) : "r"(x), "r"(y));
    return result;
}

__declare_extern_inline(float)
fmaxf(float x, float y)
{
    float result;
    if (issignaling(x) || issignaling(y))
        return x + y;

    __asm__("%0 = sfmax(%1, %2)" : "=r"(result) : "r"(x), "r"(y));
    return result;
}

__declare_extern_inline(float)
fminf(float x, float y)
{
    float result;
    if (issignaling(x) || issignaling(y))
        return x + y;

    __asm__("%0 = sfmin(%1, %2)" : "=r"(result) : "r"(x), "r"(y));
    return result;
}

__declare_extern_inline(float)
sqrtf(float x)
{
    extern float __hexagon_sqrtf(float);
#ifdef __MATH_ERRNO
    if (isless(x, 0.0f))
        errno = EDOM;
#endif
    return __hexagon_sqrtf(x);
}

#endif /* __declare_extern_inline */

#endif /* _MACHINE_MATH_H_ */
