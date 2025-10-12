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

#if defined(HAS_BINARY32) && !defined(SKIP_BINARY32)

static TEST_CONST struct {
    int         x1;
    binary32    x2;
    binary32    y;
    ulp_t       ulp;
} test_32_vec[] = {
#define REAL(r32,r64,r80,r128)          r32
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
    ulp_t       math_ulp_binary32 = math_find_ulp_binary32();

    max_ulp = 0;
    printf("test %s\n", MATH_STRING(TEST_FUNC_32));
    for (i = 0; i < count(test_32_vec); i++) {
        if (SKIP_DENORM32(test_32_vec[i].x2)) {
            printf("Skipping denorm value\n");
            continue;
        }
        binary32        y = TEST_FUNC_32(test_32_vec[i].x1, test_32_vec[i].x2);
        ulp_t           ulp = ulp32(y, test_32_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary32) {
            ret = 0;
            printf("%5zu %5d " FMT32 " got " FMT32 " want " FMT32 " ulp %" PRIdULP "\n",
                   i + 1,
                   test_32_vec[i].x1, P32(test_32_vec[i].x2),
                   P32(y),
                   P32(test_32_vec[i].y),
                   ulp);
        }
    }
    printf("  max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}
#else
#define test_binary32() 1
#endif

#if defined(HAS_BINARY64) && !defined(SKIP_BINARY64)
static TEST_CONST struct {
    int         x1;
    binary64    x2;
    binary64    y;
    ulp_t       ulp;
} test_64_vec[] = {
#define REAL(r32,r64,r80,r128)          r64
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
    ulp_t       math_ulp_binary64 = math_find_ulp_binary64();

    max_ulp = 0;
    printf("test %s\n", MATH_STRING(TEST_FUNC_64));
    for (i = 0; i < count(test_64_vec); i++) {
        if (SKIP_DENORM64(test_64_vec[i].x2)) {
            printf("Skipping denorm value\n");
            continue;
        }
        binary64        y = TEST_FUNC_64(test_64_vec[i].x1, test_64_vec[i].x2);
        ulp_t           ulp = ulp64(y, test_64_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary64) {
            ret = 0;
            printf("%5zu %d " FMT64 " got " FMT64 " want " FMT64 " ulp %" PRIdULP "\n",
                   i + 1,
                   test_64_vec[i].x1,
                   P64(test_64_vec[i].x2),
                   P64(y),
                   P64(test_64_vec[i].y),
                   ulp);
        }
    }
    printf("  max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}

#else
#define test_binary64() 1
#endif

#if defined(HAS_BINARY80) && !defined(SKIP_BINARY80)
static TEST_CONST struct {
    int         x1;
    binary80    x2;
    binary80    y;
    ulp_t       ulp;
} test_80_vec[] = {
#define REAL(r32,r64,r80,r128)          r80
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
    ulp_t       math_ulp_binary80 = math_find_ulp_binary80();

    max_ulp = 0;
    if (skip_binary80())
        return ret;
    printf("test %s\n", MATH_STRING(TEST_FUNC_80));
    for (i = 0; i < count(test_80_vec); i++) {
        if (SKIP_DENORM80(test_80_vec[i].x2)) {
            printf("Skipping denorm value\n");
            continue;
        }
        binary80        y = TEST_FUNC_80(test_80_vec[i].x1, test_80_vec[i].x2);
        ulp_t           ulp = ulp80(y, test_80_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary80) {
            ret = 0;
            printf("%5zu %d " FMT80 " got " FMT80 " want " FMT80 " ulp %" PRIdULP "\n",
                   i + 1,
                   test_80_vec[i].x1,
                   P80(test_80_vec[i].x2),
                   P80(y),
                   P80(test_80_vec[i].y),
                   ulp);
        }
    }
    printf("  max ulp %" PRIdULP "\n", max_ulp);
    return ret;
}

#else
#define test_binary80() 1
#endif

#if defined(HAS_BINARY128) && !defined(SKIIP_BINARY128)
static TEST_CONST struct {
    int         x1;
    binary128   x2;
    binary128   y;
    ulp_t       ulp;
} test_128_vec[] = {
#define REAL(r32,r64,r80,r128)          r128
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
    ulp_t       math_ulp_binary128 = math_find_ulp_binary128();

    max_ulp = 0;
    printf("test %s\n", MATH_STRING(TEST_FUNC_128));
    for (i = 0; i < count(test_128_vec); i++) {
        if (SKIP_DENORM128(test_128_vec[i].x2)) {
            printf("Skipping denorm value\n");
            continue;
        }
        binary128       y = TEST_FUNC_128(test_128_vec[i].x1, test_128_vec[i].x2);
        ulp_t           ulp = ulp128(y, test_128_vec[i].y);
        if (ulp > max_ulp)
            max_ulp = ulp;
        if (ulp > math_ulp_binary128) {
            ret = 0;
            printf("%5zu %d " FMT128 " got " FMT128 " want " FMT128 " ulp %" PRIdULP "\n",
                   i + 1,
                   test_128_vec[i].x1,
                   P128(test_128_vec[i].x2),
                   P128(y),
                   P128(test_128_vec[i].y),
                   ulp);
        }
    }
    printf("  max ulp %" PRIdULP "\n", max_ulp);
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
