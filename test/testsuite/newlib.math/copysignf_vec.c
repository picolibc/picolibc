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
 one_line_type copysignf_vec[] = {

/* copysign(x,±0) = ±x for any x (even a zero or quiet NaN) */
{64, 0,123,__LINE__, 0x7ff80000, 0x00000000, 0x7ff80000, 0x00000000, 0x00000000, 0x00000000 },   /* qnan=f(qnan, +0) */
{64, 0,123,__LINE__, 0x7ff40000, 0x00000000, 0x7ff40000, 0x00000000, 0x00000000, 0x00000000 },   /* snan=f(snan, +0) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0x7ff00000, 0x00000000, 0x00000000, 0x00000000 },   /* +inf=f(+inf, +0) */
{64, 0,123,__LINE__, 0x7ff00000, 0x00000000, 0xfff00000, 0x00000000, 0x00000000, 0x00000000 },   /* +inf=f(-inf, +0) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },   /* +0=f(+0, +0) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x80000000, 0x00000000, 0x00000000, 0x00000000 },   /* +0=f(-0, +0) */
{64, 0,123,__LINE__, 0xfff80000, 0x00000000, 0x7ff80000, 0x00000000, 0x80000000, 0x00000000 },   /* -qnan=f(qnan, -0) */
{64, 0,123,__LINE__, 0xfff40000, 0x00000000, 0x7ff40000, 0x00000000, 0x80000000, 0x00000000 },   /* -qnan=f(snan, -0) */
{64, 0,123,__LINE__, 0xfff00000, 0x00000000, 0x7ff00000, 0x00000000, 0x80000000, 0x00000000 },   /* -inf=f(+inf, -0) */
{64, 0,123,__LINE__, 0xfff00000, 0x00000000, 0xfff00000, 0x00000000, 0x80000000, 0x00000000 },   /* -inf=f(-inf, -0) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0x00000000 },   /* -0=f(+0, -0) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000 },   /* -0=f(-0, -0) */

/* copysign(±0, -y) = -0 */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0xc0080000, 0x00000000 },   /* -0=f(+0, -3) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0xc0080000, 0x00000000 },   /* -0=f(-0, -3) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0xc0100000, 0x00000000 },   /* -0=f(+0, -4) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0xc0100000, 0x00000000 },   /* -0=f(-0, -4) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0xbfe00000, 0x00000000 },   /* -0=f(+0, -0.5) */
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0xbfe00000, 0x00000000 },   /* -0=f(-0, -0.5) */

/* copysign(±0, +y) = +0 */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40080000, 0x00000000 },   /* -0=f(+0, 3) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x80000000, 0x00000000, 0x40080000, 0x00000000 },   /* -0=f(-0, 3) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x40100000, 0x00000000 },   /* -0=f(+0, 4) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x80000000, 0x00000000, 0x40100000, 0x00000000 },   /* -0=f(-0, 4) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x3fe00000, 0x00000000 },   /* -0=f(+0, 0.5) */
{64, 0,123,__LINE__, 0x00000000, 0x00000000, 0x80000000, 0x00000000, 0x3fe00000, 0x00000000 },   /* -0=f(-0, 0.5) */

/* copysign(+1,+y)= +1 for any y (even a NaN) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x7ff80000, 0x00000000 },   /* 1=f(1, qnan) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x7ff40000, 0x00000000 },   /* 1=f(1, snan) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x7ff00000, 0x00000000 },   /* 1=f(1, +inf) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x40100000, 0x00000000 },   /* 1=f(1, +4) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x3fe00000, 0x00000000 },   /* 1=f(1, +0.5) */

/* copysign(+1,+y)= -1 for any y (even a NaN) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xfff80000, 0x00000000 },   /* 1=f(1, -qnan) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xfff40000, 0x00000000 },   /* 1=f(1, -snan) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xfff00000, 0x00000000 },   /* 1=f(1, -inf) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xc0100000, 0x00000000 },   /* 1=f(1, -4) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0x3ff00000, 0x00000000, 0xbfe00000, 0x00000000 },   /* 1=f(1, -0.5) */

/* copysign(-1,+y)= +1 for any y (even a NaN) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xbff00000, 0x00000000, 0x7ff80000, 0x00000000 },   /* 1=f(1, qnan) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xbff00000, 0x00000000, 0x7ff40000, 0x00000000 },   /* 1=f(1, snan) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xbff00000, 0x00000000, 0x7ff00000, 0x00000000 },   /* 1=f(1, +inf) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xbff00000, 0x00000000, 0x40100000, 0x00000000 },   /* 1=f(1, +4) */
{64, 0,123,__LINE__, 0x3ff00000, 0x00000000, 0xbff00000, 0x00000000, 0x3fe00000, 0x00000000 },   /* 1=f(1, +0.5) */

/* copysign(+1,+y)= -1 for any y (even a NaN) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0xbff00000, 0x00000000, 0xfff80000, 0x00000000 },   /* 1=f(1, -qnan) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0xbff00000, 0x00000000, 0xfff40000, 0x00000000 },   /* 1=f(1, -snan) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0xbff00000, 0x00000000, 0xfff00000, 0x00000000 },   /* 1=f(1, -inf) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0xbff00000, 0x00000000, 0xc0100000, 0x00000000 },   /* 1=f(1, -4) */
{64, 0,123,__LINE__, 0xbff00000, 0x00000000, 0xbff00000, 0x00000000, 0xbfe00000, 0x00000000 },   /* 1=f(1, -0.5) */

{0}
};
void test_copysignf(int m)   {run_vector_1(m,copysignf_vec,(char *)(copysignf),"copysignf","fff");   }
