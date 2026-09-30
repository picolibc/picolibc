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

#define _ISOC23_SOURCE
#include "math_two.h"

#ifdef float_t

#ifdef MAX
#define BASE_PART fmaximum
#define CMP(x, y) ((x) > (y))
#else
#define BASE_PART fminimum
#define CMP(x, y) ((x) < (y))
#endif

#ifdef NUM
#define NUM_PART _num
#else
#define NUM_PART
#endif

#ifdef MAG
#define MAG_PART _mag
#define VAL(x)   name(fabs)(x)
#else
#define MAG_PART
#define VAL(x) (x)
#endif

#define _cat3(a, b, c) a##b##c
#define cat3(a, b, c)  _cat3(a, b, c)
#define NAME           cat3(BASE_PART, MAG_PART, NUM_PART)

float_t
name(NAME)(float_t x, float_t y)
{
#ifdef NUM
    if (issignaling(x))
        x = x + y;

    if (issignaling(y))
        y = x + y;

    if (isnan(y))
        return x;

    if (isnan(x))
        return y;
#else
    if (isnan(x) || isnan(y))
        return x + y;
#endif
    if (x == y)
        return CMP(!signbit(x), !signbit(y)) ? x : y;
    return CMP(VAL(x), VAL(y)) ? (x) : (y);
}

#endif
