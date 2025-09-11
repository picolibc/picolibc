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

#include <test-math.h>

#ifdef HAS_BINARY32

static struct {
    binary32   x, y;
    int32_t    ulp;
} test_32_vec[] = {
#define REAL(r32,r64,r80,r128)          FN32(r32)
#define REAL_ULP(r32,r64,r80,r128)      r32
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary32(void)
{
    size_t      i;
    int         ret = 1;
    int32_t     max_ulp = 0;

    printf("test %s\n", MATH_STRING(TEST_FUNC_32));
    for (i = 0; i < count(test_32_vec); i++) {
        binary32       y = TEST_FUNC_32(test_32_vec[i].x);
        int32_t        ulp = ulp32(y, test_32_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > test_32_vec[i].ulp) {
            ret = 0;
            printf("%5zu " FMT32 " got " FMT32 " want " FMT32 " ulp %" PRId32 "\n",
                   i + 1,
                   P32(test_32_vec[i].x),
                   P32(y),
                   P32(test_32_vec[i].y),
                   ulp);
        }
    }
    printf("  max ulp %" PRId32 "\n", max_ulp);
    return ret;
}
#else
#define test_binary32() 1
#endif

#ifdef HAS_BINARY64
static struct {
    binary64   x, y;
    int64_t    ulp;
} test_64_vec[] = {
#define REAL(r32,r64,r80,r128)          FN64(r64)
#define REAL_ULP(r32,r64,r80,r128)      r64
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary64(void)
{
    size_t      i;
    int         ret = 1;
    int64_t     max_ulp = 0;

    printf("test %s\n", MATH_STRING(TEST_FUNC_64));
    for (i = 0; i < count(test_64_vec); i++) {
        binary64       y = TEST_FUNC_64(test_64_vec[i].x);
        int64_t        ulp = ulp64(y, test_64_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > test_64_vec[i].ulp) {
            ret = 0;
            printf("%5zu " FMT64 " got " FMT64 " want " FMT64 " ulp %" PRId64 "\n",
                   i + 1,
                   P64(test_64_vec[i].x),
                   P64(y),
                   P64(test_64_vec[i].y),
                   ulp);
        }
    }
    printf("  max ulp %" PRId64 "\n", max_ulp);
    return ret;
}

#else
#define test_binary64() 1
#endif

#ifdef HAS_BINARY80
static struct {
    binary80   x, y;
    int128_t    ulp;
} test_80_vec[] = {
#define REAL(r32,r64,r80,r128)          FN80(r80)
#define REAL_ULP(r32,r64,r80,r128)      r80
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary80(void)
{
    size_t      i;
    int         ret = 1;
    int128_t    max_ulp = 0;

    printf("test %s\n", MATH_STRING(TEST_FUNC_80));
    for (i = 0; i < count(test_80_vec); i++) {
        binary80        y = TEST_FUNC_80(test_80_vec[i].x);
        int128_t        ulp = ulp80(y, test_80_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > test_80_vec[i].ulp) {
            ret = 0;
            printf("%5zu " FMT80 " got " FMT80 " want " FMT80 " ulp %" PRId128 "\n",
                   i + 1,
                   P80(test_80_vec[i].x),
                   P80(y),
                   P80(test_80_vec[i].y),
                   U128(ulp));
        }
    }
    printf("  max ulp %" PRId128 "\n", U128(max_ulp));
    return ret;
}

#else
#define test_binary80() 1
#endif

#ifdef HAS_BINARY128
static struct {
    binary128   x, y;
    int128_t    ulp;
} test_128_vec[] = {
#define REAL(r32,r64,r80,r128)          FN128(r128)
#define REAL_ULP(r32,r64,r80,r128)      r128
#include TEST_VECTORS
#undef REAL
#undef REAL_ULP
};

static int
test_binary128(void)
{
    size_t      i;
    int         ret = 1;
    int128_t     max_ulp = 0;

    printf("test %s\n", MATH_STRING(TEST_FUNC_128));
    for (i = 0; i < count(test_128_vec); i++) {
        binary128       y = TEST_FUNC_128(test_128_vec[i].x);
        int128_t        ulp = ulp128(y, test_128_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > test_128_vec[i].ulp) {
            ret = 0;
            printf("%5zu " FMT128 " got " FMT128 " want " FMT128 " ulp %" PRId128 "\n",
                   i + 1,
                   P128(test_128_vec[i].x),
                   P128(y),
                   P128(test_128_vec[i].y),
                   U128(ulp));
        }
    }
    printf("  max ulp %" PRId128 "\n", U128(max_ulp));
    return ret;
}

#else
#define test_binary128() 1
#endif

int main(void)
{
    int ret = 0;
    if (!test_binary32())
        ret = 1;
    if (!test_binary64())
        ret = 1;
    if (!test_binary80())
        ret = 1;
    if (!test_binary128())
        ret = 1;
    return ret;
}
