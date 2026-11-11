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

/*
 * Check sqrt in every rounding mode the target supports, including
 * whether it raises FE_INEXACT, for each floating-point format.
 */

#define TEST_FUNC sqrt
#include "test-math.h"

#ifdef __sh__
#if !(defined(__SH4__) || defined(__SH4_SINGLE__) || defined(__SH4_SINGLE_ONLY__))
/* The GDB simulator doesn't support floating-point exceptions */
#define GDB_SIMULATOR
#endif
#endif

/* RX fails to generate NaN, so accept any result where one is expected */
#ifdef __RX__
#define NANOK(y) 1
#else
#define NANOK(y) isnan(y)
#endif

/* Whether each format can raise exceptions on this target */
#ifdef BINARY32_NOEXCEPT
#define EXCEPT_32 0
#else
#define EXCEPT_32 1
#endif
#ifdef BINARY64_NOEXCEPT
#define EXCEPT_64 0
#else
#define EXCEPT_64 1
#endif
#ifdef BINARY80_NOEXCEPT
#define EXCEPT_80 0
#else
#define EXCEPT_80 1
#endif
#ifdef BINARY128_NOEXCEPT
#define EXCEPT_128 0
#else
#define EXCEPT_128 1
#endif

/* Which expected result each rounding mode uses */
enum { RN, RU, RD, RZ };

static const struct {
    int         mode;
    int         result;
    const char *name;
} modes[] = {
#ifdef FE_TONEAREST
    { FE_TONEAREST,  RN, "tonearest"  },
#endif
#ifdef FE_UPWARD
    { FE_UPWARD,     RU, "upward"     },
#endif
#ifdef FE_DOWNWARD
    { FE_DOWNWARD,   RD, "downward"   },
#endif
#ifdef FE_TOWARDZERO
    { FE_TOWARDZERO, RZ, "towardzero" },
#endif
};

#define NMODES (sizeof(modes) / sizeof(modes[0]))

/* Whether FE_INEXACT can be raised and tested on this target */
static int
have_inexact(void)
{
#if defined(FE_INEXACT) && !defined(GDB_SIMULATOR)
    feclearexcept(FE_INEXACT);
    feraiseexcept(FE_INEXACT);
    int ret = fetestexcept(FE_INEXACT) != 0;
    feclearexcept(FE_INEXACT);
    return ret;
#else
    return 0;
#endif
}

static void
set_nearest(void)
{
#ifdef FE_TONEAREST
    fesetround(FE_TONEAREST);
#endif
}

static int
test_inexact(void)
{
#ifdef FE_INEXACT
    return fetestexcept(FE_INEXACT) != 0;
#else
    return 0;
#endif
}

/*
 * The test for one format, using the vec_N table. Results must match
 * exactly, including the sign of zero.
 */
#define TEST_FORMAT(N, T, FUNC, FMT, P, SKIP_DENORM, SKIP)                                      \
    static T want_##N(size_t i, int result)                                                     \
    {                                                                                           \
        switch (result) {                                                                       \
        default:                                                                                \
            return vec_##N[i].rn;                                                               \
        case RU:                                                                                \
            return vec_##N[i].ru;                                                               \
        case RD:                                                                                \
            return vec_##N[i].rd;                                                               \
        case RZ:                                                                                \
            return vec_##N[i].rz;                                                               \
        }                                                                                       \
    }                                                                                           \
                                                                                                \
    static int test_##N(void)                                                                   \
    {                                                                                           \
        T      (     *volatile f)(T) = FUNC;                                                        \
        int    errors = 0;                                                                      \
        int    check_inexact = EXCEPT_##N && have_inexact();                                    \
        int    tested = 0;                                                                      \
        size_t m, i;                                                                            \
                                                                                                \
        if (SKIP)                                                                               \
            return 0;                                                                           \
        /*                                                                                      \
         * One pass per rounding mode the target can set. If it can't                           \
         * set any, check the round-to-nearest results in the default                           \
         * mode instead.                                                                        \
         */                                                                                     \
        for (m = 0; m <= NMODES; m++) {                                                         \
            int         result = RN;                                                            \
            const char *name = "default";                                                       \
            if (m < NMODES) {                                                                   \
                if (fesetround(modes[m].mode) != 0)                                             \
                    continue;                                                                   \
                result = modes[m].result;                                                       \
                name = modes[m].name;                                                           \
            } else if (tested) {                                                                \
                break;                                                                          \
            }                                                                                   \
            tested = 1;                                                                         \
            printf("test %s %s\n", MATH_STRING(FUNC), name);                                    \
            for (i = 0; i < sizeof(vec_##N) / sizeof(vec_##N[0]); i++) {                        \
                volatile T x = vec_##N[i].x;                                                    \
                T          want = want_##N(i, result);                                          \
                if (SKIP_DENORM(x))                                                             \
                    continue;                                                                   \
                feclearexcept(FE_ALL_EXCEPT);                                                   \
                volatile T y = f(x);                                                            \
                int        inexact = test_inexact();                                            \
                int ok = isnan(want) ? NANOK(y) : y == want && !!signbit(y) == !!signbit(want); \
                if (check_inexact && inexact != vec_##N[i].inexact)                             \
                    ok = 0;                                                                     \
                if (!ok) {                                                                      \
                    if (errors < 10)                                                            \
                        printf("%5zu " FMT " got " FMT " want " FMT " inexact %d want %d\n",    \
                               i + 1, P(x), P(y), P(want), inexact, vec_##N[i].inexact);        \
                    errors++;                                                                   \
                }                                                                               \
            }                                                                                   \
        }                                                                                       \
        set_nearest();                                                                          \
        return errors;                                                                          \
    }

#define VEC(N, T)              \
    static const struct {      \
        T   x, rn, ru, rd, rz; \
        int inexact;           \
    } vec_##N[]

#if defined(HAS_BINARY32)
#define REAL(r32, r64, r80, r80m, r128) r32
VEC(32, binary32) = {
#include "test-sqrt.h"
};
#undef REAL
TEST_FORMAT(32, binary32, TEST_FUNC_32, FMT32, P32, SKIP_DENORM32, 0)
#else
#define test_32() 0
#endif

#if defined(HAS_BINARY64)
#define REAL(r32, r64, r80, r80m, r128) r64
VEC(64, binary64) = {
#include "test-sqrt.h"
};
#undef REAL
TEST_FORMAT(64, binary64, TEST_FUNC_64, FMT64, P64, SKIP_DENORM64, 0)
#else
#define test_64() 0
#endif

#if defined(HAS_BINARY80)
#define REAL(r32, r64, r80, r80m, r128) REAL80(r80, r80m)
VEC(80, binary80) = {
#include "test-sqrt.h"
};
#undef REAL
TEST_FORMAT(80, binary80, TEST_FUNC_80, FMT80, P80, SKIP_DENORM80, skip_binary80())
#else
#define test_80() 0
#endif

#if defined(HAS_BINARY128)
#define REAL(r32, r64, r80, r80m, r128) r128
VEC(128, binary128) = {
#include "test-sqrt.h"
};
#undef REAL
TEST_FORMAT(128, binary128, TEST_FUNC_128, FMT128, P128, SKIP_DENORM128, 0)
#else
#define test_128() 0
#endif

int
main(void)
{
    int errors = test_32() + test_64() + test_80() + test_128();

    if (errors)
        printf("%d errors\n", errors);
    return errors != 0;
}
