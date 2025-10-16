/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#ifndef TEST_FUNC
#error Pass -DTEST_FUNC=function when building
#endif

#define _GNU_SOURCE
#include "../test-math/test-math.h"

#define FNAME   MATH_STRING(TEST_FUNC)

#if !defined(HAS_BINARY32) || !defined(HAS_BINARY64)
int main(void)
{
    printf("Skipping test-math-compare-" FNAME " on target without 32-bit and 64-bit floats\n");
    return(77);
}

#else

static inline binary32
binary32_from_uint32(uint32_t u32)
{
    union {
        binary32           f32;
        uint32_t        u32;
    } u;

    u.u32 = u32;
    return u.f32;
}

static inline uint32_t
uint32_from_binary32(binary32 f32)
{
    union {
        binary32           f32;
        uint32_t        u32;
    } u;

    u.f32 = f32;
    return u.u32;
}

#define UINV_ULP        UINTMAX_C(0xffffffff)

static inline uint32_t
compare_ulp32(binary32 x, binary32 y)
{
    uint32_t    ulp = 0;
    uint32_t    ux = uint32_from_binary32(x);
    uint32_t    uy = uint32_from_binary32(y);

    if (ux == uy)
        return 0;

    if (isnan(x) && isnan(y))
        return 0;
    if (isnan(x) || isnan(y))
        return UINV_ULP;

    if ((ux & 0x80000000UL) != (uy & 0x80000000UL))
        ulp++;

    ux &= 0x7fffffffUL;
    uy &= 0x7fffffffUL;
    if (ux > uy)
        ulp += ux - uy;
    else
        ulp += uy - ux;
    return ulp;
}

#ifdef __PICOLIBC__
#define LIBNAME "picolibc"
#else
#define LIBNAME "native"
#endif

int main(void)
{
    uint32_t    u32 = 0x00000000UL;
    uint32_t    max_ulp = 0;
    uint32_t    max_ulp_u32 = 0;
    ulp_t       check_ulps = math_find_ulp_binary32();
    int         ret = 0;

    printf("%s: ", FNAME);
    fflush(stdout);
    for (;;) {
        binary32        fx = binary32_from_uint32(u32);
        binary64        dx = (binary64) fx;
        binary32        fy = TEST_FUNC_32(fx);
        binary64        dy = TEST_FUNC_64(dx);
        binary32        fdy = (binary32) dy;

        uint32_t        ulp = compare_ulp32(fy, fdy);
        if (ulp > max_ulp) {
            max_ulp = ulp;
            max_ulp_u32 = u32;
            if (ulp > (uint32_t) check_ulps && check_ulps < MAX_ULP) {
                ret = 1;
                printf("%s: ulp %" PRIu32 " 0x%08" PRIx32 " %a got 0x%08" PRIx32 " %a want 0x%08" PRIx32 " %a\n",
                       FNAME, ulp,
                       uint32_from_binary32(fx), (binary64) fx,
                       uint32_from_binary32(fy), (binary64) fy,
                       uint32_from_binary32(fdy), (binary64) fdy);
            }
        }

        if ((u32 & 0xfffffff) == 0) {
            printf(".");
            fflush(stdout);
        }

        if (u32 == 0xffffffffUL)
            break;
        u32++;
    }
    printf("\n");
    printf("%s %s: max_ulp %" PRIu32 " at 0x%08" PRIx32 " %a\n",
           LIBNAME, FNAME, max_ulp, max_ulp_u32, (binary64) binary32_from_uint32(max_ulp_u32));
    if (ret)
        printf("FAILED\n");
    else
        printf("PASSED\n");
    return ret;
}
#endif
