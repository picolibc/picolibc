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

#include "test-math.h"

#ifdef HAS_BINARY32

static TEST_CONST struct {
    cbinary32   x, y;
} test_32_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX32(r32,i32)
#include TEST_VECTORS
#undef COMPLEX
};

static int
test_binary32(void)
{
    size_t      i;
    int         ret = 1;
    ulp_t       math_ulp_binary32 = math_find_ulp_binary32();

    max_ulp = 0;
    printf("test %s\n", MATH_STRING(TEST_FUNC_32));
    for (i = 0; i < count(test_32_vec); i++) {
        if (SKIP_CDENORM32(test_32_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        cbinary32       y = TEST_FUNC_32(test_32_vec[i].x);
        ulp_t          ulp = culp32(y, test_32_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary32) {
            ret = 0;
            printf("%5zu " FMT32 " + " FMT32 " i got " FMT32 " + " FMT32 " i want " FMT32 " + " FMT32 " i ulp %" PRIdULP "\n",
                   i + 1,
                   P32(creal32(test_32_vec[i].x)), P32(cimag32(test_32_vec[i].x)),
                   P32(creal32(y)), P32(cimag32(y)),
                   P32(creal32(test_32_vec[i].y)), P32(cimag32(test_32_vec[i].y)),
                   ulp);
        }
    }
    printf("max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}
#else
#define test_binary32() 1
#endif

#ifdef HAS_BINARY64
static TEST_CONST struct {
    cbinary64   x, y;
} test_64_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX64(r64,i64)
#include TEST_VECTORS
#undef COMPLEX
};

static int
test_binary64(void)
{
    size_t      i;
    int         ret = 1;
    ulp_t       math_ulp_binary64 = math_find_ulp_binary64();

    max_ulp = 0;
    printf("test %s\n", MATH_STRING(TEST_FUNC_64));
    for (i = 0; i < count(test_64_vec); i++) {
        if (SKIP_CDENORM64(test_64_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        cbinary64       y = TEST_FUNC_64(test_64_vec[i].x);
        ulp_t           ulp = culp64(y, test_64_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary64) {
            ret = 0;
            printf("%5zu " FMT64 " + " FMT64 " i got " FMT64 " + " FMT64 " i want " FMT64 " + " FMT64 " i ulp %" PRIdULP "\n",
                   i + 1,
                   P64(creal64(test_64_vec[i].x)), P64(cimag64(test_64_vec[i].x)),
                   P64(creal64(y)), P64(cimag64(y)),
                   P64(creal64(test_64_vec[i].y)), P64(cimag64(test_64_vec[i].y)),
                   ulp);
        }
    }
    printf("max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}

#else
#define test_binary64() 1
#endif

#ifdef HAS_BINARY80
static TEST_CONST struct {
    cbinary80   x, y;
} test_80_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX80(r80,i80)
#include TEST_VECTORS
#undef COMPLEX
};

static int
test_binary80(void)
{
    size_t      i;
    int         ret = 1;
    ulp_t       math_ulp_binary80 = math_find_ulp_binary80();

    max_ulp = 0;
    if (skip_binary80())
        return ret;
    printf("test %s\n", MATH_STRING(TEST_FUNC_80));
    for (i = 0; i < count(test_80_vec); i++) {
        if (SKIP_CDENORM80(test_80_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        cbinary80       y = TEST_FUNC_80(test_80_vec[i].x);
        ulp_t           ulp = culp80(y, test_80_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary80) {
            ret = 0;
            printf("%5zu " FMT80 " + " FMT80 " i got " FMT80 " + " FMT80 " i want " FMT80 " + " FMT80 " i ulp %" PRIdULP "\n",
                   i + 1,
                   P80(creal80(test_80_vec[i].x)), P80(cimag80(test_80_vec[i].x)),
                   P80(creal80(y)), P80(cimag80(y)),
                   P80(creal80(test_80_vec[i].y)), P80(cimag80(test_80_vec[i].y)),
                   ulp);
        }
    }
    printf("max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}

#else
#define test_binary80() 1
#endif

#ifdef HAS_BINARY128
static TEST_CONST struct {
    cbinary128   x, y;
} test_128_vec[] = {
#define COMPLEX(r32,i32,r64,i64,r80,i80,r128,i128) CMPLX128(r128,i128)
#include TEST_VECTORS
#undef COMPLEX
};

static int
test_binary128(void)
{
    size_t      i;
    int         ret = 1;
    ulp_t       math_ulp_binary128 = math_find_ulp_binary128();

    max_ulp = 0;
    printf("test %s\n", MATH_STRING(TEST_FUNC_128));
    for (i = 0; i < count(test_128_vec); i++) {
        if (SKIP_CDENORM128(test_128_vec[i].x)) {
            printf("Skipping denorm value\n");
            continue;
        }
        cbinary128       y = TEST_FUNC_128(test_128_vec[i].x);
        ulp_t           ulp = culp128(y, test_128_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary128) {
            ret = 0;
            printf("%5zu " FMT128 " + " FMT128 " i got " FMT128 " + " FMT128 " i want " FMT128 " + " FMT128 " i ulp %" PRIdULP "\n",
                   i + 1,
                   P128(creal128(test_128_vec[i].x)), P128(cimag128(test_128_vec[i].x)),
                   P128(creal128(y)), P128(cimag128(y)),
                   P128(creal128(test_128_vec[i].y)), P128(cimag128(test_128_vec[i].y)),
                   ulp);
        }
    }
    printf("max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}

#else
#define test_binary128() 1
#endif

int main(void)
{
    const char spaces[] = "               ";

    int ret = 0;
    max_ulp = math_find_ulp_binary32();
    if (!test_binary32())
        ret = 1;
    ulp_t max_ulp_32 = max_ulp;
    max_ulp = math_find_ulp_binary64();
    if (!test_binary64())
        ret = 1;
    ulp_t max_ulp_64 = max_ulp;
    max_ulp = math_find_ulp_binary80();
    if (!test_binary80())
        ret = 1;
    ulp_t max_ulp_80 = max_ulp;
    max_ulp = math_find_ulp_binary128();
    if (!test_binary128())
        ret = 1;
    ulp_t max_ulp_128 = max_ulp;

    size_t len = strlen(MATH_STRING(TEST_FUNC));
    size_t want_len = 10;
    size_t extra = want_len - len;
    const char *space = spaces + strlen(spaces) - extra;
    assert(strlen(space) == extra);
    printf("    { .name = \"%s\",%s .b32 = %7" PRIdULP ", .b64 = %7" PRIdULP ", .b80 = %7" PRIdULP ", .b128 = %7" PRIdULP " },\n",
           MATH_STRING(TEST_FUNC), space, max_ulp_32, max_ulp_64, max_ulp_80, max_ulp_128);
    return ret;
}
