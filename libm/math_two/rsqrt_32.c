/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2025-2026 Dominik Kurz
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
#define WANT_FLOAT32
#include "math_two.h"
#include <stdbit.h>

#define QUIET_NAN    (((uint32_t)255u << 23) | ((uint32_t)1u << 22) | 1u)
#define INF          ((uint32_t)255u << 23)
#define NEGATIVE_INF (((uint32_t)1u << 31) | ((uint32_t)255u << 23))

// hard float rsqrtf
#if !(__OBSOLETE_MATH_FLOAT)
float
rsqrtf(float x)
{
    return 1.0 / sqrt((double)x);
}

#else

static uint32_t
sqrt_core(uint32_t xyy, uint32_t y)
{
    xyy <<= 6;
    uint32_t t = xyy + (xyy >> 1);
    if (t < ((uint32_t)1u << 31)) { // first iteration is special cased
        t += t >> 1;
        if (t < ((uint32_t)1u << 31)) {
            xyy = t;
            y += y >> 1;
        }
    }
#ifndef __OPTIMIZE_SIZE__
#pragma GCC unroll 12
#endif
    for (uint32_t i = 2; i < 14; i += 1) {
#if 1
        uint32_t t = xyy + (xyy >> i);
        t += t >> i;
        if (t < ((uint32_t)1u << 31)) {
            xyy = t;
            y += y >> i;
        }
#else
        // optimal sequence for arm32, GCC 14 manages it, GCC 15/16 and clang don't
        asm(".syntax unified\n\t"
            "adds   r12,  %[xyy], %[xyy], lsr %[i]\n\t"
            "addspl r12,  r12,  r12,  lsr %[i]\n\t"
            "movpl  %[xyy], r12 \n\t"
            "addpl  %[y], %[y], %[y], lsr %[i]"
            : [xyy] "+&r"(xyy), [y] "+&r"(y) // output
            : [i] "rI"(i)                    // input
            : "r12", "cc"                    // clobber
        );
#endif
    }
    // below is identical to a Newton iteration
    // y=y*(3/2-xyy/2)
    // it can be rewritten as y+(y*((1-xyy)/2)
    // so that (1-xyy)/2 is contained in the lower 16-bits
    // and the lowest 16-bits of y get dropped
    // but this requires increasing the number of loop iterations
    // so it is generally not a big win unless 32x32->64 multiply is slow
    // or there is an optimized helper for 32x16->48 bit multiply
    uint32_t hi = ((uint64_t)xyy * y) >> 32;
    y += y >> 1;   // this can overflow
    return y - hi; // but then this underflows so they cancel
}

static uint32_t
mantissa_rsqrt(uint32_t x)
{
    uint32_t Y = x;
    uint32_t U = sqrt_core(x, (uint32_t)1u << 30) >> 6;
    // we could return (U+1)>>1 and be 90 % accurate
    // but we want infinitely precise rounding
    U |= 1;
    uint64_t A = (uint64_t)U * U;
    uint32_t A_hi = A >> 32;
    uint32_t A_lo = A;
    uint32_t B_hi = ((uint64_t)Y * A_lo) >> 32;
    uint64_t C = (uint64_t)Y * A_hi;
    uint64_t S = B_hi + C;
    uint32_t P = S >> 32;
    uint32_t Q = 1u << 9;
    if (P < Q)
        U += 1;

    return U >> 1;
}

// soft float rsqrtf. May still be faster than hard float if double support is slow. Requires
// testing.
float
rsqrtf(float x)
{
    uint32_t xu = asuint(x);
    int32_t  exp = asr((int32_t)xu, 23);

    if (likely(exp > 0)) {
        if (unlikely(exp == 255)) {
            // 1 / sqrt(+Inf) = +0
            // 1 / sqrt(NaN) = qNaN
            if (xu == INF) {
                return asfloat(0);
            } else {
                __math_set_invalidf();
                // pass through NaN unmodified
                return asfloat(xu);
            }
        } else // normal case
        {
            uint32_t mantissa = xu & ~((uint32_t)exp << 23);
            mantissa += (uint32_t)1u << 23;
            mantissa <<= !(exp & 1);
            uint32_t newMantissa = mantissa_rsqrt(mantissa);
            int32_t  newExp = -asr(exp - 127, 1) + 127 - 2;
            return asfloat(newMantissa + ((uint32_t)newExp << 23));
        }
    } else // exp<=0
    {
        if (likely(exp == 0)) {
            if (likely(xu != 0)) // subnormal
            {
                uint32_t mantissa = xu;
                int32_t  shift = stdc_leading_zeros(mantissa) - (31 - 23);
                mantissa <<= shift; // normalize subnormal
                shift = -126 - shift;
                mantissa <<= shift & 1;
                int32_t  newExp = -asr(shift, 1) + 127 - 2;
                uint32_t new_mantissa = mantissa_rsqrt(mantissa);
                return asfloat(new_mantissa + ((uint32_t)newExp << 23));
            } else // rsqrt(+0)=+INF
            {
#ifdef __MATH_ERRNO
                errno = ERANGE;
#endif
                return asfloat(INF);
            }
        } else if (xu == ((uint32_t)1u << 31)) {
#ifdef __MATH_ERRNO
            errno = ERANGE;
#endif
            return asfloat(NEGATIVE_INF);
        } else {
            __math_set_invalidf();
#ifdef __MATH_ERRNO
            errno = EDOM;
#endif
            // rsqrt(negative)=qNaN
            return nanf("");
        }
    }
}
#endif

_MATH_ALIAS_f_f(rsqrt)

// sqrtf implementation, can be enabled on soft-float targets
#if 0
static uint32_t mantissa_sqrt(uint32_t x)
{
    uint32_t new_mantissa=sqrt_core(x,x<<7 )>>8;
    //we could stop here but we want infinitely precise rounding
    int64_t msquared=(int64_t)new_mantissa*new_mantissa;
    int64_t x0=x;
    x0<<=23;
    int64_t residual=x0-msquared;
    if (residual>new_mantissa)
            new_mantissa+=1;

    return new_mantissa;
}

float sqrtf(float x)
{
    uint32_t xu=asuint(x);
    int32_t exponent= asr((int32_t )xu,23);
    // check if exponent is 0
    if (likely(exponent>0) )
    {
        if (unlikely( exponent==255 ) ) //check if negative or NaN
        {
            // Expected behavior:
            // sqrt(-f)=+qNaN, sqrt(-NaN)=+qNaN,sqrt(-Inf)=+qNaN
            if (xu == INF)
            {
                return asfloat(INF);
            }
            else
            {
                __math_set_invalidf();
                return asfloat(xu);
            }
        }
        else
        {
            uint32_t mantissa = xu &~ ((uint32_t)exponent << 23);
            exponent = exponent - (127);
            mantissa += 1 << 23; // adds implicit bit to mantissa.
            mantissa <<= (exponent & 1);
            exponent =asr(exponent,1);
            // This is meant to be a floor division
            // meaning -1/2= -0.5 should map to -1
            exponent = exponent + 126;
            uint32_t new_mantissa=mantissa_sqrt(mantissa);
            return asfloat(((uint32_t)exponent<<23) + new_mantissa);
        }
    }
    else
    {
        if (likely(exponent == 0))
        {
            if (likely (xu !=0) )
            {
                uint32_t mantissa = xu &~ ((uint32_t)exponent << 23);
                int32_t shift=stdc_leading_zeros(mantissa)- (31-23);
                mantissa <<= shift; // normalize subnormal
                int32_t exponent =  -126-shift;
                mantissa <<= (exponent & 1);
                uint32_t new_mantissa=mantissa_sqrt(mantissa);
                exponent =asr(exponent,1);
                exponent = (exponent + (126));
                return asfloat(((uint32_t)exponent<<23) + new_mantissa);
            } else //sqrt(+0)=+0
            {
                return asfloat(0);
            }

        } else if (xu == ((uint32_t)1u<<31))
        {
            //sqrt(-0)=-0
            return asfloat((uint32_t)1u<<31);
        } else {
            __math_set_invalidf();
#ifdef __MATH_ERRNO
            errno = EDOM;
#endif
            //sqrt(negative)=qNaN
            return nanf("");
        }

    }
}
#endif
