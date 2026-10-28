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

#ifndef _TGMATH_H_
#define _TGMATH_H_

#define __STDC_VERSION_TGMATH_H__ 202311L

#include <math.h>

#ifndef __cplusplus

#include <complex.h>

/*
 * Type-generic math macros (C23 7.27).
 *
 * __tg_type maps each generic argument to a zero of the type used to
 * select the function: integer arguments are treated as double. The
 * sum of these values for all generic arguments has the type the
 * standard selects, as the usual arithmetic conversions pick the
 * widest real type and make the result complex if any argument is
 * complex.
 *
 * The _Generic controlling expressions are not evaluated, so each
 * argument is evaluated exactly once, in the function call.
 */

#define __tg_type(__x)                                 \
    _Generic((__x),                                    \
        float: (float)0,                               \
        long double: (long double)0,                   \
        float _Complex: (float _Complex)0,             \
        double _Complex: (double _Complex)0,           \
        long double _Complex: (long double _Complex)0, \
        default: (double)0)

#define __tg_type2(__x, __y)      (__tg_type(__x) + __tg_type(__y))
#define __tg_type3(__x, __y, __z) (__tg_type(__x) + __tg_type(__y) + __tg_type(__z))

/*
 * Without long double math functions, long double arguments use the
 * double functions.
 */
#ifdef __HAVE_LONG_DOUBLE_MATH
#define __tg_l(__f) __f##l
#else
#define __tg_l(__f) __f
#endif

/* Real-only functions */
#define __tg_r(__e, __f) _Generic((__e), float: __f##f, long double: __tg_l(__f), default: __f)

/* Functions with both real and complex versions */
#define __tg_rc(__e, __f, __cf)             \
    _Generic((__e),                         \
        float: __f##f,                      \
        long double: __tg_l(__f),           \
        float _Complex: __cf##f,            \
        double _Complex: __cf,              \
        long double _Complex: __tg_l(__cf), \
        default: __f)

/* Complex-only functions; real arguments use the corresponding complex type */
#define __tg_c(__e, __cf)                   \
    _Generic((__e),                         \
        float: __cf##f,                     \
        float _Complex: __cf##f,            \
        long double: __tg_l(__cf),          \
        long double _Complex: __tg_l(__cf), \
        default: __cf)

/* Narrowing functions returning float: long double or double arguments */
#define __tg_nf(__e, __f)             _Generic((__e), long double: __tg_l(__f), default: __f)

#define __tg_r1(__f, __x)             __tg_r(__tg_type(__x), __f)(__x)
#define __tg_r2(__f, __x, __y)        __tg_r(__tg_type2(__x, __y), __f)(__x, __y)
#define __tg_rc1(__f, __cf, __x)      __tg_rc(__tg_type(__x), __f, __cf)(__x)
#define __tg_rc2(__f, __cf, __x, __y) __tg_rc(__tg_type2(__x, __y), __f, __cf)(__x, __y)
#define __tg_c1(__cf, __x)            __tg_c(__tg_type(__x), __cf)(__x)

/* 7.27 paragraph 4: functions with real and complex versions */
#define acos(__x)     __tg_rc1(acos, cacos, __x)
#define asin(__x)     __tg_rc1(asin, casin, __x)
#define atan(__x)     __tg_rc1(atan, catan, __x)
#define acosh(__x)    __tg_rc1(acosh, cacosh, __x)
#define asinh(__x)    __tg_rc1(asinh, casinh, __x)
#define atanh(__x)    __tg_rc1(atanh, catanh, __x)
#define cos(__x)      __tg_rc1(cos, ccos, __x)
#define sin(__x)      __tg_rc1(sin, csin, __x)
#define tan(__x)      __tg_rc1(tan, ctan, __x)
#define cosh(__x)     __tg_rc1(cosh, ccosh, __x)
#define sinh(__x)     __tg_rc1(sinh, csinh, __x)
#define tanh(__x)     __tg_rc1(tanh, ctanh, __x)
#define exp(__x)      __tg_rc1(exp, cexp, __x)
#define log(__x)      __tg_rc1(log, clog, __x)
#define pow(__x, __y) __tg_rc2(pow, cpow, __x, __y)
#define sqrt(__x)     __tg_rc1(sqrt, csqrt, __x)
/*
 * Convert integer arguments to double explicitly; compilers warn
 * about passing integers to fabs. Floating arguments are unchanged.
 */
#define fabs(__x) __tg_rc(__tg_type(__x), fabs, cabs)((__typeof__(__tg_type(__x)))(__x))

/* 7.27 paragraph 5: real-only functions */
#define atan2(__y, __x)       __tg_r2(atan2, __y, __x)
#define cbrt(__x)             __tg_r1(cbrt, __x)
#define ceil(__x)             __tg_r1(ceil, __x)
#define copysign(__x, __y)    __tg_r2(copysign, __x, __y)
#define erf(__x)              __tg_r1(erf, __x)
#define erfc(__x)             __tg_r1(erfc, __x)
#define exp2(__x)             __tg_r1(exp2, __x)
#define expm1(__x)            __tg_r1(expm1, __x)
#define fdim(__x, __y)        __tg_r2(fdim, __x, __y)
#define floor(__x)            __tg_r1(floor, __x)
#define fma(__x, __y, __z)    __tg_r(__tg_type3(__x, __y, __z), fma)(__x, __y, __z)
#define fmax(__x, __y)        __tg_r2(fmax, __x, __y)
#define fmin(__x, __y)        __tg_r2(fmin, __x, __y)
#define fmod(__x, __y)        __tg_r2(fmod, __x, __y)
#define frexp(__x, __e)       __tg_r(__tg_type(__x), frexp)(__x, __e)
#define hypot(__x, __y)       __tg_r2(hypot, __x, __y)
#define ilogb(__x)            __tg_r1(ilogb, __x)
#define ldexp(__x, __e)       __tg_r(__tg_type(__x), ldexp)(__x, __e)
#define lgamma(__x)           __tg_r1(lgamma, __x)
#define llrint(__x)           __tg_r1(llrint, __x)
#define llround(__x)          __tg_r1(llround, __x)
#define log10(__x)            __tg_r1(log10, __x)
#define log1p(__x)            __tg_r1(log1p, __x)
#define log2(__x)             __tg_r1(log2, __x)
#define logb(__x)             __tg_r1(logb, __x)
#define lrint(__x)            __tg_r1(lrint, __x)
#define lround(__x)           __tg_r1(lround, __x)
#define nearbyint(__x)        __tg_r1(nearbyint, __x)
#define nextafter(__x, __y)   __tg_r2(nextafter, __x, __y)
#define nexttoward(__x, __y)  __tg_r(__tg_type(__x), nexttoward)(__x, __y)
#define remainder(__x, __y)   __tg_r2(remainder, __x, __y)
#define remquo(__x, __y, __q) __tg_r(__tg_type2(__x, __y), remquo)(__x, __y, __q)
#define rint(__x)             __tg_r1(rint, __x)
#define round(__x)            __tg_r1(round, __x)
#define scalbln(__x, __e)     __tg_r(__tg_type(__x), scalbln)(__x, __e)
#define scalbn(__x, __e)      __tg_r(__tg_type(__x), scalbn)(__x, __e)
#define tgamma(__x)           __tg_r1(tgamma, __x)
#define trunc(__x)            __tg_r1(trunc, __x)

#if __GNU_VISIBLE || __ISO_C_VISIBLE >= 2023
#define acospi(__x)                __tg_r1(acospi, __x)
#define asinpi(__x)                __tg_r1(asinpi, __x)
#define atanpi(__x)                __tg_r1(atanpi, __x)
#define atan2pi(__y, __x)          __tg_r2(atan2pi, __y, __x)
#define compoundn(__x, __n)        __tg_r(__tg_type(__x), compoundn)(__x, __n)
#define cospi(__x)                 __tg_r1(cospi, __x)
#define exp10(__x)                 __tg_r1(exp10, __x)
#define exp10m1(__x)               __tg_r1(exp10m1, __x)
#define exp2m1(__x)                __tg_r1(exp2m1, __x)
#define fmaximum(__x, __y)         __tg_r2(fmaximum, __x, __y)
#define fmaximum_mag(__x, __y)     __tg_r2(fmaximum_mag, __x, __y)
#define fmaximum_num(__x, __y)     __tg_r2(fmaximum_num, __x, __y)
#define fmaximum_mag_num(__x, __y) __tg_r2(fmaximum_mag_num, __x, __y)
#define fminimum(__x, __y)         __tg_r2(fminimum, __x, __y)
#define fminimum_mag(__x, __y)     __tg_r2(fminimum_mag, __x, __y)
#define fminimum_num(__x, __y)     __tg_r2(fminimum_num, __x, __y)
#define fminimum_mag_num(__x, __y) __tg_r2(fminimum_mag_num, __x, __y)
#define fromfp(__x, __r, __w)      __tg_r(__tg_type(__x), fromfp)(__x, __r, __w)
#define fromfpx(__x, __r, __w)     __tg_r(__tg_type(__x), fromfpx)(__x, __r, __w)
#define llogb(__x)                 __tg_r1(llogb, __x)
#define log10p1(__x)               __tg_r1(log10p1, __x)
#define log2p1(__x)                __tg_r1(log2p1, __x)
#define logp1(__x)                 __tg_r1(logp1, __x)
#define nextdown(__x)              __tg_r1(nextdown, __x)
#define nextup(__x)                __tg_r1(nextup, __x)
#define pown(__x, __n)             __tg_r(__tg_type(__x), pown)(__x, __n)
#define powr(__x, __y)             __tg_r2(powr, __x, __y)
#define rootn(__x, __n)            __tg_r(__tg_type(__x), rootn)(__x, __n)
#define roundeven(__x)             __tg_r1(roundeven, __x)
#define rsqrt(__x)                 __tg_r1(rsqrt, __x)
#define sinpi(__x)                 __tg_r1(sinpi, __x)
#define tanpi(__x)                 __tg_r1(tanpi, __x)
#define ufromfp(__x, __r, __w)     __tg_r(__tg_type(__x), ufromfp)(__x, __r, __w)
#define ufromfpx(__x, __r, __w)    __tg_r(__tg_type(__x), ufromfpx)(__x, __r, __w)

/*
 * 7.27 paragraph 7: narrowing functions. f* use the long double
 * version when any argument is long double, otherwise the double
 * version. d* only exist with long double arguments.
 */
#define fadd(__x, __y)      __tg_nf(__tg_type2(__x, __y), fadd)(__x, __y)
#define fsub(__x, __y)      __tg_nf(__tg_type2(__x, __y), fsub)(__x, __y)
#define fmul(__x, __y)      __tg_nf(__tg_type2(__x, __y), fmul)(__x, __y)
#define fdiv(__x, __y)      __tg_nf(__tg_type2(__x, __y), fdiv)(__x, __y)
#define ffma(__x, __y, __z) __tg_nf(__tg_type3(__x, __y, __z), ffma)(__x, __y, __z)
#define fsqrt(__x)          __tg_nf(__tg_type(__x), fsqrt)(__x)

#ifdef __HAVE_LONG_DOUBLE_MATH
#define dadd(__x, __y)      daddl(__x, __y)
#define dsub(__x, __y)      dsubl(__x, __y)
#define dmul(__x, __y)      dmull(__x, __y)
#define ddiv(__x, __y)      ddivl(__x, __y)
#define dfma(__x, __y, __z) dfmal(__x, __y, __z)
#define dsqrt(__x)          dsqrtl(__x)
#endif
#endif /* __GNU_VISIBLE || __ISO_C_VISIBLE >= 2023 */

/* 7.27 paragraph 6: complex-only functions */
#define carg(__x)  __tg_c1(carg, __x)
#define cimag(__x) __tg_c1(cimag, __x)
#define conj(__x)  __tg_c1(conj, __x)
#define cproj(__x) __tg_c1(cproj, __x)
#define creal(__x) __tg_c1(creal, __x)

#endif /* !__cplusplus */

#endif /* _TGMATH_H_ */
