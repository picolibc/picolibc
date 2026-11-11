/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Check that math.h works in C++, where the C23 classification macros
 * and the picolibc suffixed ones are overloaded functions instead.
 * scripts/check-cplusplus-macros also compiles this with each C++
 * version and checks that math.h doesn't define the macros listed in
 * cplusplus-hidden-macros.txt.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <math.h>
#include <float.h>
#include <stdio.h>

/*
 * Since C++11, <cmath> declares bool isinf(double) and bool
 * isnan(double), and the C++ library may bring them into the global
 * namespace, so math.h must not declare int versions of them
 */
#if __cplusplus >= 201103L
#undef isinf
#undef isnan
namespace cxx {
bool isinf(double);
bool isnan(double);
}
using cxx::isinf;
using cxx::isnan;
#endif

/* Like test-math-errhandling, skip these where loading a value quiets signaling NaNs */
#if !defined(__mcffpu__) && !defined(__HAVE_68881__) && !(defined(__i386__) && !defined(__SSE_MATH__))
#define TEST_SNAN
#endif

/* With -ffinite-math-only, as used for RX, the compiler may assume there are no infinities or NaNs */
#if !__FINITE_MATH_ONLY__
#define TEST_NAN_INF
#endif

#if defined(_TEST_LONG_DOUBLE) && defined(__HAVE_LONG_DOUBLE_MATH)
#define TEST_LONG_DOUBLE
#endif

static int failures;

static void
check(bool ok, const char *expr, const char *type, int line)
{
    if (!ok) {
        printf("%s:%d: %s failed for %s\n", __FILE__, line, expr, type);
        ++failures;
    }
}

#define CHECK(expr) check((expr) != 0, #expr, type, __LINE__)

/* Check the functions with finite values of one floating type */
template <class T>
static void
check_finite(const char *type, T min)
{
    volatile T zero = 0, one = 1, tiny = min;
    volatile T neg_zero = -zero;

    tiny = tiny / 2;

    CHECK(iszero(zero));
    CHECK(iszero(neg_zero));
    CHECK(!iszero(one));
    CHECK(!iszero(min));

    CHECK(!issubnormal(zero));
    CHECK(!issubnormal(one));
    CHECK(!issubnormal(min));
    /* Targets which flush subnormals to zero get zero here */
    if (tiny != 0)
        CHECK(issubnormal(tiny));

    CHECK(!issignaling(zero));
    CHECK(!issignaling(one));

    CHECK(iscanonical(zero));
    CHECK(iscanonical(one));

    CHECK(iseqsig(one, one));
    CHECK(iseqsig(zero, neg_zero));
    CHECK(!iseqsig(zero, one));
}

#ifdef TEST_NAN_INF
/* Check the functions with infinity and NaNs of one floating type */
template <class T>
static void
check_nan_inf(const char *type, T inf, T qnan, T snan)
{
    CHECK(!iszero(inf));
    CHECK(!iszero(qnan));
    CHECK(!issubnormal(inf));
    CHECK(!issubnormal(qnan));
    CHECK(!issignaling(inf));
    CHECK(!issignaling(qnan));
#ifdef TEST_SNAN
    CHECK(issignaling(snan));
#else
    (void)snan;
#endif
    CHECK(iscanonical(inf));
}
#endif

/* iseqsig compares in the wider of the two types */
static void
check_mixed(void)
{
    const char     *type = "mixed types";
    volatile float  f_one = 1.0f;
    volatile double d_one = 1.0, d_next = 1.0 + DBL_EPSILON;

    CHECK(iseqsig(f_one, d_one));
    CHECK(iseqsig(d_one, f_one));
#if DBL_MANT_DIG > FLT_MANT_DIG
    CHECK(!iseqsig(f_one, d_next));
    CHECK(!iseqsig(d_next, f_one));
#else
    (void)d_next;
#endif
#ifdef TEST_LONG_DOUBLE
    volatile long double l_one = 1.0L, l_next = 1.0L + LDBL_EPSILON;

    CHECK(iseqsig(d_one, l_one));
#if LDBL_MANT_DIG > DBL_MANT_DIG
    CHECK(!iseqsig(d_one, l_next));
#else
    (void)l_next;
#endif
#endif
}

static void
check_suffixed(void)
{
    const char    *type = "suffixed functions";
    volatile float f_zero = 0.0f, f_one = 1.0f;

    CHECK(isfinitef(f_one));
    CHECK(isnormalf(f_one));
    CHECK(!isnormalf(f_zero));
    CHECK(iszerof(f_zero));
    CHECK(!iszerof(f_one));
#ifdef TEST_NAN_INF
    CHECK(!isfinitef(__builtin_inff()));
    CHECK(isinff(__builtin_inff()));
    CHECK(isnanf(__builtin_nanf("")));
#endif
#ifdef TEST_LONG_DOUBLE
    volatile long double l_zero = 0.0L, l_one = 1.0L;

    CHECK(isfinitel(l_one));
    CHECK(isnormall(l_one));
    CHECK(!isnormall(l_zero));
    CHECK(iszerol(l_zero));
    CHECK(!iszerol(l_one));
#ifdef TEST_NAN_INF
    CHECK(!isfinitel(__builtin_infl()));
    CHECK(isinfl(__builtin_infl()));
    CHECK(isnanl(__builtin_nanl("")));
#endif
#endif
}

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wmain"
#endif

extern "C" {

int main(void);

int
main(void)
{
    check_finite<float>("float", FLT_MIN);
    check_finite<double>("double", DBL_MIN);
#ifdef TEST_NAN_INF
    check_nan_inf<float>("float", __builtin_inff(), __builtin_nanf(""), __builtin_nansf(""));
    check_nan_inf<double>("double", __builtin_inf(), __builtin_nan(""), __builtin_nans(""));
#endif
#ifdef TEST_LONG_DOUBLE
    check_finite<long double>("long double", LDBL_MIN);
#ifdef TEST_NAN_INF
    check_nan_inf<long double>("long double", __builtin_infl(), __builtin_nanl(""),
                               __builtin_nansl(""));
#endif
#endif
    check_mixed();
    check_suffixed();
    return failures != 0;
}

}
