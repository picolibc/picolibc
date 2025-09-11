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
    cbinary32   x, y;
    cint32_t    ulp;
} test_32_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX32(r32,i32)
#define COMPLEX_ULP(r32,i32,r64,i64,r80,i80,r128,i128) .r = r32, .a = i32
#include TEST_VECTORS
#undef COMPLEX
#undef COMPLEX_ULP
};

static int
test_binary32(void)
{
    size_t      i;
    int         ret = 1;
    cint32_t    max_ulp = {};

    printf("test %s\n", MATH_STRING(TEST_FUNC_32));
    for (i = 0; i < count(test_32_vec); i++) {
        cbinary32       y = TEST_FUNC_32(test_32_vec[i].x);
        cint32_t        ulp = culp32(y, test_32_vec[i].y);
        if (ulp.r > max_ulp.r)
            max_ulp.r = ulp.r;
        if (ulp.a > max_ulp.a)
            max_ulp.a = ulp.a;
        if (ulp.r > test_32_vec[i].ulp.r || ulp.a > test_32_vec[i].ulp.a) {
            ret = 0;
            printf("%5zu " FMT32 " + " FMT32 " i got " FMT32 " + " FMT32 " i want " FMT32 " + " FMT32 " i ulp %" PRId32 "/%" PRId32 "\n",
                   i + 1,
                   P32(creal32(test_32_vec[i].x)), P32(cimag32(test_32_vec[i].x)),
                   P32(creal32(y)), P32(cimag32(y)),
                   P32(creal32(test_32_vec[i].y)), P32(cimag32(test_32_vec[i].y)),
                   ulp.r, ulp.a);
        }
    }
    printf("max ulp %" PRId32 "/%" PRId32 "\n", max_ulp.r, max_ulp.a);
    return ret;
}
#else
#define test_binary32() 1
#endif

#ifdef HAS_BINARY64
static struct {
    cbinary64   x, y;
    cint64_t    ulp;
} test_64_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX64(r64,i64)
#define COMPLEX_ULP(r32,i32,r64,i64,r80,i80,r128,i128) .r = r64, .a = i64
#include TEST_VECTORS
#undef COMPLEX
#undef COMPLEX_ULP
};

static int
test_binary64(void)
{
    size_t      i;
    int         ret = 1;
    cint64_t    max_ulp = {};

    printf("test %s\n", MATH_STRING(TEST_FUNC_64));
    for (i = 0; i < count(test_64_vec); i++) {
        cbinary64       y = TEST_FUNC_64(test_64_vec[i].x);
        cint64_t        ulp = culp64(y, test_64_vec[i].y);
        if (ulp.r > max_ulp.r)
            max_ulp.r = ulp.r;
        if (ulp.a > max_ulp.a)
            max_ulp.a = ulp.a;
        if (ulp.r > test_64_vec[i].ulp.r || ulp.a > test_64_vec[i].ulp.a) {
            ret = 0;
            printf("%5zu " FMT64 " + " FMT64 " i got " FMT64 " + " FMT64 " i want " FMT64 " + " FMT64 " i ulp %" PRId64 "/%" PRId64 "\n",
                   i + 1,
                   P64(creal64(test_64_vec[i].x)), P64(cimag64(test_64_vec[i].x)),
                   P64(creal64(y)), P64(cimag64(y)),
                   P64(creal64(test_64_vec[i].y)), P64(cimag64(test_64_vec[i].y)),
                   ulp.r, ulp.a);
        }
    }
    printf("max ulp %" PRId64 "/%" PRId64 "\n", max_ulp.r, max_ulp.a);
    return ret;
}

#else
#define test_binary64() 1
#endif

#ifdef HAS_BINARY80
static struct {
    cbinary80   x, y;
    cint128_t    ulp;
} test_80_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX80(r80,i80)
#define COMPLEX_ULP(r32,i32,r64,i64,r80,i80,r128,i128) .r = r80, .a = i80
#include TEST_VECTORS
#undef COMPLEX
#undef COMPLEX_ULP
};

static int
test_binary80(void)
{
    size_t      i;
    int         ret = 1;
    cint128_t    max_ulp = {};

    printf("test %s\n", MATH_STRING(TEST_FUNC_80));
    for (i = 0; i < count(test_80_vec); i++) {
        cbinary80       y = TEST_FUNC_80(test_80_vec[i].x);
        cint128_t        ulp = culp80(y, test_80_vec[i].y);
        if (ulp.r > max_ulp.r)
            max_ulp.r = ulp.r;
        if (ulp.a > max_ulp.a)
            max_ulp.a = ulp.a;
        if (ulp.r > test_80_vec[i].ulp.r || ulp.a > test_80_vec[i].ulp.a) {
            ret = 0;
            printf("%5zu " FMT80 " + " FMT80 " i got " FMT80 " + " FMT80 " i want " FMT80 " + " FMT80 " i ulp %" PRId128 "/%" PRId128 "\n",
                   i + 1,
                   P80(creal80(test_80_vec[i].x)), P80(cimag80(test_80_vec[i].x)),
                   P80(creal80(y)), P80(cimag80(y)),
                   P80(creal80(test_80_vec[i].y)), P80(cimag80(test_80_vec[i].y)),
                   U128(ulp.r), U128(ulp.a));
        }
    }
    printf("max ulp %" PRId128 "/%" PRId128 "\n", U128(max_ulp.r), U128(max_ulp.a));
    return ret;
}

#else
#define test_binary80() 1
#endif

#ifdef HAS_BINARY128
static struct {
    cbinary128   x, y;
    cint128_t    ulp;
} test_128_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX128(r128,i128)
#define COMPLEX_ULP(r32,i32,r64,i64,r80,i80,r128,i128) .r = r128, .a = i128
#include TEST_VECTORS
#undef COMPLEX
#undef COMPLEX_ULP
};

static int
test_binary128(void)
{
    size_t      i;
    int         ret = 1;
    cint128_t    max_ulp = {};

    printf("test %s\n", MATH_STRING(TEST_FUNC_128));
    for (i = 0; i < count(test_128_vec); i++) {
        cbinary128       y = TEST_FUNC_128(test_128_vec[i].x);
        cint128_t        ulp = culp128(y, test_128_vec[i].y);
        if (ulp.r > max_ulp.r)
            max_ulp.r = ulp.r;
        if (ulp.a > max_ulp.a)
            max_ulp.a = ulp.a;
        if (ulp.r > test_128_vec[i].ulp.r || ulp.a > test_128_vec[i].ulp.a) {
            ret = 0;
            printf("%5zu " FMT128 " + " FMT128 " i got " FMT128 " + " FMT128 " i want " FMT128 " + " FMT128 " i ulp %" PRId128 "/%" PRId128 "\n",
                   i + 1,
                   P128(creal128(test_128_vec[i].x)), P128(cimag128(test_128_vec[i].x)),
                   P128(creal128(y)), P128(cimag128(y)),
                   P128(creal128(test_128_vec[i].y)), P128(cimag128(test_128_vec[i].y)),
                   U128(ulp.r), U128(ulp.a));
        }
    }
    printf("max ulp %" PRId128 "/%" PRId128 "\n", U128(max_ulp.r), U128(max_ulp.a));
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
