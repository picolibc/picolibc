/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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

#define _HAVE_FAST_FMA 1
#define _HAVE_FAST_FMAF 1

#ifdef __declare_extern_inline

#ifdef _WANT_MATH_ERRNO
#include <errno.h>
#endif

__declare_extern_inline(double)
sqrt (double x)
{
    double result;
#ifdef _WANT_MATH_ERRNO
    if (isless(x, 0.0))
        errno = EDOM;
#endif
    __asm__ __volatile__ ("fsqrt\t%d0, %d1" : "=w" (result) : "w" (x));
    return result;
}

__declare_extern_inline(float)
sqrtf (float x)
{
    float result;
#ifdef _WANT_MATH_ERRNO
    if (isless(x, 0.0f))
        errno = EDOM;
#endif
    __asm__ __volatile__ ("fsqrt\t%s0, %s1" : "=w" (result) : "w" (x));
    return result;
}

__declare_extern_inline(double)
fma (double x, double y, double z)
{
    double result;
    __asm__ __volatile__ ("fmadd\t%d0, %d1, %d2, %d3" : "=w" (result) : "w" (x), "w" (y), "w" (z));
    return result;
}

__declare_extern_inline(float)
fmaf (float x, float y, float z)
{
    float result;
    __asm__ __volatile__ ("fmadd\t%s0, %s1, %s2, %s3" : "=w" (result) : "w" (x), "w" (y), "w" (z));
    return result;
}

#endif

#endif /* _MACHINE_MATH_H_ */
