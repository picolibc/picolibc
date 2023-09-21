/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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
#include "test.h"
 one_line_type pow_vec[] = {

#ifdef __mcffpu__
#define SKIP_SNAN_CHECKS
#endif

/* pow(x,±0) = 1 for any x (even a zero or quiet NaN) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x7ff80000, 0x00000000, 0x00000000, 0x00000000 },   /* 1=f(qnan, +0) */
#ifndef SKIP_SNAN_CHECKS
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000, 0x00000000, 0x00000000 },   /* qnan=f(snan, +0) */
#endif
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x7ff00000, 0x00000000, 0x00000000, 0x00000000 },   /* 1=f(+inf, +0) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xfff00000, 0x00000000, 0x00000000, 0x00000000 },   /* 1=f(-inf, +0) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },   /* 1=f(+0, +0) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x80000000, 0x00000000, 0x00000000, 0x00000000 },   /* 1=f(-0, +0) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x7ff80000, 0x00000000, 0x80000000, 0x00000000 },   /* 1=f(qnan, -0) */
#ifndef SKIP_SNAN_CHECKS
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000, 0x80000000, 0x00000000 },   /* qnan=f(snan, -0) */
#endif
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x7ff00000, 0x00000000, 0x80000000, 0x00000000 },   /* 1=f(+inf, -0) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xfff00000, 0x00000000, 0x80000000, 0x00000000 },   /* 1=f(-inf, -0) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0x00000000 },   /* 1=f(+0, -0) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000 },   /* 1=f(-0, -0) */

/* pow(±0, y) = ±∞ and signals divideByZero for y an odd integer < 0 */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x00000000, 0x00000000, 0xc0080000, 0x00000000 },   /* +inf=f(+0, -3) */
{64, 0,123,__LINE__, 0xfff00000, 0x00000000, 0x80000000, 0x00000000, 0xc0080000, 0x00000000 },   /* -inf=f(-0, -3) */

/* pow(±0, y) = +∞ and signals divideByZero for y < 0 and not an odd integer */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x00000000, 0x00000000, 0xc0100000, 0x00000000 },   /* +inf=f(+0, -4) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x80000000, 0x00000000, 0xc0100000, 0x00000000 },   /* -inf=f(-0, -4) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x00000000, 0x00000000, 0xbfe00000, 0x00000000 },   /* +inf=f(+0, -0.5) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x80000000, 0x00000000, 0xbfe00000, 0x00000000 },   /* +inf=f(-0, -0.5) */

/* pow(±0, y)= +0 for y >0 and not an odd integer */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40100000, 0x00000000 },   /* +inf=f(+0, 4) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x80000000, 0x00000000, 0x40100000, 0x00000000 },   /* -inf=f(-0, 4) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x3fe00000, 0x00000000 },   /* +inf=f(+0, 0.5) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x80000000, 0x00000000, 0x3fe00000, 0x00000000 },   /* +inf=f(-0, 0.5) */

/* pow(+1,y)= +1 for any y (even a quiet NaN) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x7ff80000, 0x00000000 },   /* 1=f(1, qnan) */
#ifndef SKIP_SNAN_CHECKS
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x3ff00000, 0x00000000, 0x7ff40000, 0x00000000 },   /* qnan=f(1, snan) */
#endif
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x7ff00000, 0x00000000 },   /* 1=f(1, +inf) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xfff00000, 0x00000000 },   /* 1=f(1, -inf) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x40100000, 0x00000000 },   /* 1=f(1, +4) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xc0100000, 0x00000000 },   /* 1=f(1, -4) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x3fe00000, 0x00000000 },   /* 1=f(1, +0.5) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xbfe00000, 0x00000000 },   /* 1=f(1, -0.5) */

/* pow(x,y) returns a quiet NaN and signals invalid for finite x < 0 and finite non-integer y */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0xbff00000, 0x00000000, 0xbfe00000, 0x00000000 },   /* 1=f(-1, -0.5) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0xc0100000, 0x00000000, 0xbfe00000, 0x00000000 },   /* 1=f(-4, -0.5) */

/* except for the above cases, nan operands return qnan */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000 },   /* qnan=f(qnan, qnan) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000, 0x7ff80000, 0x00000000 },   /* qnan=f(snan, qnan) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000 },   /* qnan=f(qnan, snan) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000, 0x7ff40000, 0x00000000 },   /* qnan=f(snan, snan) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000 },   /* qnan=f(qnan, qnan) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000, 0x3ff00000, 0x00000000 },   /* qnan=f(qnan, 1) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000, 0x3ff00000, 0x00000000 },   /* qnan=f(snan, 1) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000, 0xbff00000, 0x00000000 },   /* qnan=f(qnan, -1) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff40000, 0x00000000, 0xbff00000, 0x00000000 },   /* qnan=f(snan, -1) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0xbff00000, 0x00000000, 0x7ff80000, 0x00000000 },   /* qnan=f(-1, qnan) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0xbff00000, 0x00000000, 0x7ff40000, 0x00000000 },   /* qnan=f(-1, snan) */

/* If x is +0 (-0), and y is an odd integer greater than 0, the result is +0 (-0). */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40080000, 0x00000000 },   /* +0=f(+0, 3) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x40080000, 0x00000000 },   /* -0=f(-0, 3) */

/* If x is 0, and y greater than 0 and not an odd integer, the result is +0. */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40100000, 0x00000000 },   /* +0=f(+0, 4) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x80000000, 0x00000000, 0x40100000, 0x00000000 },   /* +0=f(-0, 4) */

/* If x is -1, and y is positive infinity or negative infinity, the result is 1.0. */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xbff00000, 0x00000000, 0x7ff00000, 0x00000000 },   /* 1=f(-1, +inf) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xbff00000, 0x00000000, 0xfff00000, 0x00000000 },   /* 1=f(-1, -inf) */

/* If the absolute value of x is less than 1, and y is negative infinity, the result is positive infinity. */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x3fe00000, 0x00000000, 0xfff00000, 0x00000000 },   /* +inf=f(0.5, -inf) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0xbfe00000, 0x00000000, 0xfff00000, 0x00000000 },   /* +inf=f(-0.5, -inf) */

/* If the absolute value of x is greater than 1, and y is negative infinity, the result is +0. */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x40080000, 0x00000000, 0xfff00000, 0x00000000 },   /* +0=f(3.0, -inf) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0xc0080000, 0x00000000, 0xfff00000, 0x00000000 },   /* +0=f(-3.0, -inf) */

/* If the absolute value of x is less than 1, and y is positive infinity, the result is +0. */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x3fe00000, 0x00000000, 0x7ff00000, 0x00000000 },   /* +0=f(0.5, +inf) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0xbfe00000, 0x00000000, 0x7ff00000, 0x00000000 },   /* +0=f(-0.5, +inf) */

/* If the absolute value of x is greater than 1, and y is positive infinity, the result is positive infinity. */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x40080000, 0x00000000, 0x7ff00000, 0x00000000 },   /* +inf=f(3.0, inf) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0xc0080000, 0x00000000, 0x7ff00000, 0x00000000 },   /* +inf=f(-3.0, inf) */

/* If x is negative infinity, and y is an odd integer less than 0, the result is -0. */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0xfff00000, 0x00000000, 0xc0080000, 0x00000000 },   /* -0=f(-inf, -3) */

/* If x is negative infinity, and y less than 0 and not an odd integer, the  result  is +0. */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0xfff00000, 0x00000000, 0xc0100000, 0x00000000 },   /* +0=f(-inf, -4) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0xfff00000, 0x00000000, 0xbfe00000, 0x00000000 },   /* +0=f(-inf, -0.5) */

/* If  x  is  negative  infinity, and y is an odd integer greater than 0, the result is negative infinity. */
{64, 0,123,__LINE__, 0xfff00000, 0x00000000, 0xfff00000, 0x00000000, 0x40080000, 0x00000000 },   /* -inf=f(-inf, 3) */

/* If x is negative infinity, and y greater than 0 and not an odd integer,  the  result is positive infinity. */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0xfff00000, 0x00000000, 0x40100000, 0x00000000 },   /* +inf=f(-inf, 4) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0xfff00000, 0x00000000, 0x3fe00000, 0x00000000 },   /* +inf=f(-inf, 4) */

/* If x is positive infinity, and y less than 0, the result is +0. */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x7ff00000, 0x00000000, 0xc0080000, 0x00000000 },   /* +0=f(+inf, -3) */

/* If x is positive infinity, and y greater than 0, the result is positive infinity. */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x7ff00000, 0x00000000, 0x40100000, 0x00000000 },   /* +inf=f(+inf, 4) */

/* x is close to one and y is large */
{64, 0,123,__LINE__, 0x792ffffe, 0x0bc9e399, 0x3ff00000, 0x2c5e2e99, 0x41ec9eee, 0x35374af6},
{64, 0,123,__LINE__, 0x18bfffff, 0xec16bafd, 0x3fefffff, 0xd2e3e669, 0x41f344c9, 0x823eb66c},

{0},};
void test_pow_vec(int m)   {run_vector_1(m,pow_vec,(char *)(pow),"pow","ddd");   }
