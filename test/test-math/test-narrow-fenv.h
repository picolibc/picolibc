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
 * Rounding mode and exception checks for one family of narrowing
 * functions, included from test-narrow-fenv.c with N_T, W_T and the
 * function and limit names defined.
 */

#ifdef NO_ROUND
#define NOROUND_SKIP(m) ((m) != FE_TONEAREST)
#else
#define NOROUND_SKIP(m) 0
#endif

#ifdef NO_EXCEPT
#define NOEXCEPT_SKIP 1
#else
#define NOEXCEPT_SKIP 0
#endif

/*
 * With -ffinite-math-only, the compiler assumes there are no
 * infinities or NaNs, so checks which produce them aren't meaningful
 */
#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__
#define NONFINITE_SKIP 1
#else
#define NONFINITE_SKIP 0
#endif

static int
test_family(void)
{
    int ret = 0;

    if (SKIP_FAMILY)
        return 0;

    /* Inputs are volatile so the compiler can't evaluate the calls */
    volatile W_T one = 1, two = 2, three = 3, four = 4, quarter = (W_T)0.25;
    volatile W_T tiny = (W_T)(N_EPSILON / 128);
    volatile W_T n_max = (W_T)N_MAX, w_max = (W_T)W_MAX;
    volatile W_T n_true_min = (W_T)N_TRUE_MIN, w_true_min = (W_T)W_TRUE_MIN;
    volatile W_T zero = 0, qnan = (W_T)NAN;

    /* 1 + tiny and 1 + 3 * tiny are only exact when W_T is wide enough */
    const bool   wide = W_MANT_DIG >= N_MANT_DIG + 9;
    volatile W_T one_tiny = one + tiny, one_tiny3 = one + three * tiny;

    /*
     * Work around bug in QEMU 9.2.0: the m68k fcmp instruction raises
     * FE_INVALID for quiet NaNs
     */
    feclearexcept(FE_ALL_EXCEPT);
    volatile bool qnan_less = __builtin_isless(qnan, zero);
    const bool    qnan_compare_ok = !qnan_less && !fetestexcept(FE_INVALID);
    if (!qnan_compare_ok)
        printf("%s: comparing a quiet NaN raises FE_INVALID, skipping quiet NaN checks\n", FAMILY);
    if (NONFINITE_SKIP)
        printf("%s: finite math only, skipping infinity and NaN checks\n", FAMILY);

    const N_T n_one_up = NEXTUP_N((N_T)1);
    const N_T n_one_down = -n_one_up;

#define CHECK(desc, call, e, except)                                                              \
    do {                                                                                          \
        const expect_t _e = e;                                                                    \
        for (unsigned _m = 0; _m < NMODES; _m++) {                                                \
            if (NOROUND_SKIP(modes[_m].mode))                                                     \
                continue;                                                                         \
            fesetround(modes[_m].mode);                                                           \
            feclearexcept(FE_ALL_EXCEPT);                                                         \
            volatile N_T _r = call;                                                               \
            int          _x = fetestexcept(MY_EXCEPT);                                            \
            fesetround(FE_TONEAREST);                                                             \
            double _want = expected(&_e, modes[_m].mode);                                         \
            if (!same((double)_r, _want)) {                                                       \
                printf("%s %s %s: got %a want %a\n", FAMILY, desc, modes[_m].name, (double)_r,    \
                       _want);                                                                    \
                ret++;                                                                            \
            }                                                                                     \
            if (!NOEXCEPT_SKIP && _x != ((except) & MY_EXCEPT)) {                                 \
                printf("%s %s %s: exceptions 0x%x want 0x%x\n", FAMILY, desc, modes[_m].name, _x, \
                       (except) & MY_EXCEPT);                                                     \
                ret++;                                                                            \
            }                                                                                     \
        }                                                                                         \
    } while (0)

#define ALL(v) { (double)(v), (double)(v), (double)(v), (double)(v) }
#define MODES(near, up, down, towards)                                  \
    { (double)(near), (double)(up), (double)(down), (double)(towards) }

    /* Exact results raise nothing */
    CHECK("add 1 + 2", ADD(one, two), ALL(3.0), 0);
    CHECK("div 1 / 4", DIV(one, four), ALL(0.25), 0);
    CHECK("sqrt 4", SQRT(four), ALL(2.0), 0);
    CHECK("fma 1 * 2 + 1", FMA(one, two, one), ALL(3.0), 0);

    /* An exact zero is -0 only when rounding downward */
    CHECK("sub 1 - 1", SUB(one, one), MODES(0.0, 0.0, -0.0, 0.0), 0);
    CHECK("fma 1 * 1 - 1", FMA(one, one, -one), MODES(0.0, 0.0, -0.0, 0.0), 0);

    /* Results between two narrow values round according to the mode */
    CHECK("add 1 + tiny", ADD(one, tiny), MODES(1.0, n_one_up, 1.0, 1.0), FE_INEXACT);
    CHECK("add -1 - tiny", ADD(-one, -tiny), MODES(-1.0, -1.0, n_one_down, -1.0), FE_INEXACT);
    CHECK("sub 1 - -tiny", SUB(one, -tiny), MODES(1.0, n_one_up, 1.0, 1.0), FE_INEXACT);
    CHECK("fma 1 * 1 + tiny", FMA(one, one, tiny), MODES(1.0, n_one_up, 1.0, 1.0), FE_INEXACT);
    if (wide) {
        CHECK("mul (1 + tiny) * 1", MUL(one_tiny, one), MODES(1.0, n_one_up, 1.0, 1.0), FE_INEXACT);
        CHECK("div (1 + tiny) / 1", DIV(one_tiny, one), MODES(1.0, n_one_up, 1.0, 1.0), FE_INEXACT);
        CHECK("sqrt (1 + 3 * tiny)", SQRT(one_tiny3), MODES(1.0, n_one_up, 1.0, 1.0), FE_INEXACT);
    }

    /* Overflow gives the largest finite value when rounding toward zero */
    if (!NONFINITE_SKIP) {
        CHECK("mul max * 2", MUL(n_max, two), MODES(INFINITY, INFINITY, N_MAX, N_MAX),
              FE_OVERFLOW | FE_INEXACT);
        CHECK("mul -max * 2", MUL(-n_max, two), MODES(-INFINITY, -N_MAX, -INFINITY, -N_MAX),
              FE_OVERFLOW | FE_INEXACT);
        CHECK("mul wide max * 2", MUL(w_max, two), MODES(INFINITY, INFINITY, N_MAX, N_MAX),
              FE_OVERFLOW | FE_INEXACT);
        CHECK("add wide max + wide max", ADD(w_max, w_max), MODES(INFINITY, INFINITY, N_MAX, N_MAX),
              FE_OVERFLOW | FE_INEXACT);
        CHECK("div max / 0.25", DIV(n_max, quarter), MODES(INFINITY, INFINITY, N_MAX, N_MAX),
              FE_OVERFLOW | FE_INEXACT);
        CHECK("fma max * 2 + 0", FMA(n_max, two, zero), MODES(INFINITY, INFINITY, N_MAX, N_MAX),
              FE_OVERFLOW | FE_INEXACT);
        if (w_max > n_max)
            CHECK("sqrt wide max", SQRT(w_max), MODES(INFINITY, INFINITY, N_MAX, N_MAX),
                  FE_OVERFLOW | FE_INEXACT);
    }

    /* Tiny results round to zero or the smallest denorm */
    CHECK("mul min * 0.25", MUL(n_true_min, quarter), MODES(0.0, N_TRUE_MIN, 0.0, 0.0),
          FE_UNDERFLOW | FE_INEXACT);
    CHECK("mul -min * 0.25", MUL(-n_true_min, quarter), MODES(-0.0, -0.0, -N_TRUE_MIN, -0.0),
          FE_UNDERFLOW | FE_INEXACT);
    CHECK("div wide min / 4", DIV(w_true_min, four), MODES(0.0, N_TRUE_MIN, 0.0, 0.0),
          FE_UNDERFLOW | FE_INEXACT);
    CHECK("fma min * 0.25 + 0", FMA(n_true_min, quarter, zero), MODES(0.0, N_TRUE_MIN, 0.0, 0.0),
          FE_UNDERFLOW | FE_INEXACT);

    /* Square roots of negative values raise only invalid; quiet NaNs raise nothing */
    if (!NONFINITE_SKIP) {
        CHECK("sqrt -1", SQRT(-one), ALL(NAN), FE_INVALID);
        CHECK("sqrt -wide max", SQRT(-w_max), ALL(NAN), FE_INVALID);
        CHECK("sqrt -wide min", SQRT(-w_true_min), ALL(NAN), FE_INVALID);
    }
    if (qnan_compare_ok && !NONFINITE_SKIP) {
        CHECK("sqrt qnan", SQRT(qnan), ALL(NAN), 0);
        CHECK("add qnan", ADD(qnan, one), ALL(NAN), 0);
    }

#undef CHECK
#undef ALL
#undef MODES

    return ret;
}

#undef N_T
#undef W_T
#undef FAMILY
#undef ADD
#undef SUB
#undef MUL
#undef DIV
#undef FMA
#undef SQRT
#undef N_MAX
#undef N_TRUE_MIN
#undef N_EPSILON
#undef W_MAX
#undef W_TRUE_MIN
#undef NEXTUP_N
#undef N_MANT_DIG
#undef W_MANT_DIG
#undef NO_ROUND
#undef NO_EXCEPT
#undef NOROUND_SKIP
#undef NOEXCEPT_SKIP
#undef NONFINITE_SKIP
#undef SKIP_FAMILY
#undef test_family
