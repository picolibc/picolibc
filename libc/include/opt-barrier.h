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

#ifndef _OPT_BARRIER_H_
#define _OPT_BARRIER_H_

/*
 * Optimization barriers for library code. Compilers don't know that
 * inline asm which changes the rounding mode or exception flags
 * affects floating point operations, so they may move, reuse or
 * recompute operations across those changes.
 *
 * barrier_float(x) returns x after passing it through an asm the
 * compiler can't see into: operations using the result can't be moved
 * before the barrier, the value can't be recomputed after it, and the
 * barrier stays ordered with the fenv asm. Code that only needs x
 * computed, for example so that it raises its exceptions at that
 * point, discards the result: (void)barrier_float(x).
 *
 * barrier_int(x) does the same for integers, so the compiler can't
 * turn operations on the value into floating point operations, which
 * might raise exceptions.
 *
 * opt_barrier_float(x) and opt_barrier_int(x) are optional barriers:
 * they apply the barrier only when the target raises exceptions for
 * the type of x (for integers, the floating point type of the same
 * size), and otherwise return x unchanged.
 *
 * clang_barrier_float(x) is opt_barrier_float(x) when compiling with
 * Clang, which otherwise moves or removes operations that raise
 * exceptions, and just x with other compilers, which don't need it.
 *
 * Each macro selects a static inline function for the type of x with
 * _Generic. Each function passes the value through an empty asm, with
 * an asm constraint for its type. <machine/opt-barrier.h> may choose
 * those constraints, register classes which keep values where they
 * already are (__OPT_BARRIER_FLOAT_F, _D and _L, and __OPT_BARRIER_INT_I,
 * _UI, _L, _UL, _LL and _ULL); otherwise floating point values pass
 * through memory, which also rounds them to their type on targets with
 * extended precision registers, and integers through a general
 * register. It may instead supply a whole function (__barrier_float_f,
 * _d and _l, and __barrier_int_i, _ui, _l, _ul, _ll and _ull) along
 * with a macro of the same name. Without GNU asm, the functions store
 * the value in a volatile variable and read it back. The opt_ versions
 * are always defined here, on top of those, with an #if per type on
 * whether the target raises exceptions for it.
 *
 * Without _Generic (before C11, other than GCC 4.9 and later or
 * Clang, which accept it in any mode), the barriers do nothing. The
 * library still builds, but the compiler may then move or remove
 * operations which raise exceptions.
 *
 * This header is internal to the library and is not installed.
 */

#include <sys/cdefs.h>
#include <fenv.h>
#include <machine/opt-barrier.h>

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || __GNUC_PREREQ(4, 9) \
    || defined(__clang__)
#define __OPT_BARRIER_GENERIC_SELECTION
#endif

#ifndef __OPT_BARRIER_GENERIC_SELECTION

#define barrier_float(x)       (x)
#define barrier_int(x)         (x)
#define opt_barrier_float(x)   (x)
#define opt_barrier_int(x)     (x)
#define clang_barrier_float(x) (x)

#else

#if defined(__GNUC__)
#define __OPT_BARRIER_ASM
#endif

/*
 * asm constraints for each type, unless <machine/opt-barrier.h> chose
 * them: floating point values go through memory and integers through a
 * general register.
 */
#ifndef __OPT_BARRIER_FLOAT_F
#define __OPT_BARRIER_FLOAT_F "+m"
#endif
#ifndef __OPT_BARRIER_FLOAT_D
#define __OPT_BARRIER_FLOAT_D "+m"
#endif
#ifndef __OPT_BARRIER_FLOAT_L
#define __OPT_BARRIER_FLOAT_L "+m"
#endif
#ifndef __OPT_BARRIER_INT_I
#define __OPT_BARRIER_INT_I "+r"
#endif
#ifndef __OPT_BARRIER_INT_UI
#define __OPT_BARRIER_INT_UI "+r"
#endif
#ifndef __OPT_BARRIER_INT_L
#define __OPT_BARRIER_INT_L "+r"
#endif
#ifndef __OPT_BARRIER_INT_UL
#define __OPT_BARRIER_INT_UL "+r"
#endif
#ifndef __OPT_BARRIER_INT_LL
#define __OPT_BARRIER_INT_LL "+r"
#endif
#ifndef __OPT_BARRIER_INT_ULL
#define __OPT_BARRIER_INT_ULL "+r"
#endif

/*
 * The barrier functions, unless <machine/opt-barrier.h> supplied them.
 * With GNU asm, the value passes through an empty asm with the
 * constraint for its type; without it, through a volatile variable.
 */
#ifdef __OPT_BARRIER_ASM
#define __OPT_BARRIER_FN(name, type, constraint)    \
    static __always_inline type name(type __x)      \
    {                                               \
        __asm__ __volatile__("" : constraint(__x)); \
        return __x;                                 \
    }
#else
#define __OPT_BARRIER_FN(name, type, constraint) \
    static __always_inline type name(type __x)   \
    {                                            \
        volatile type __v = __x;                 \
        return __v;                              \
    }
#endif

#ifndef __barrier_float_f
__OPT_BARRIER_FN(__barrier_float_f, float, __OPT_BARRIER_FLOAT_F)
#endif
#ifndef __barrier_float_d
__OPT_BARRIER_FN(__barrier_float_d, double, __OPT_BARRIER_FLOAT_D)
#endif
#ifndef __barrier_float_l
__OPT_BARRIER_FN(__barrier_float_l, long double, __OPT_BARRIER_FLOAT_L)
#endif
#ifndef __barrier_int_i
__OPT_BARRIER_FN(__barrier_int_i, int, __OPT_BARRIER_INT_I)
#endif
#ifndef __barrier_int_ui
__OPT_BARRIER_FN(__barrier_int_ui, unsigned int, __OPT_BARRIER_INT_UI)
#endif
#ifndef __barrier_int_l
__OPT_BARRIER_FN(__barrier_int_l, long, __OPT_BARRIER_INT_L)
#endif
#ifndef __barrier_int_ul
__OPT_BARRIER_FN(__barrier_int_ul, unsigned long, __OPT_BARRIER_INT_UL)
#endif
#ifndef __barrier_int_ll
__OPT_BARRIER_FN(__barrier_int_ll, long long, __OPT_BARRIER_INT_LL)
#endif
#ifndef __barrier_int_ull
__OPT_BARRIER_FN(__barrier_int_ull, unsigned long long, __OPT_BARRIER_INT_ULL)
#endif

#define barrier_float(x)                   \
    _Generic((x),                          \
        float: __barrier_float_f,          \
        double: __barrier_float_d,         \
        long double: __barrier_float_l)(x)

#define barrier_int(x)                            \
    _Generic((x),                                 \
        int: __barrier_int_i,                     \
        unsigned int: __barrier_int_ui,           \
        long: __barrier_int_l,                    \
        unsigned long: __barrier_int_ul,          \
        long long: __barrier_int_ll,              \
        unsigned long long: __barrier_int_ull)(x)

/*
 * Optional barriers, applied only when the target raises exceptions for
 * the type. For each type, an #if selects either the barrier or, defined
 * only when needed, a function which returns its argument unchanged.
 */
#define __OPT_BARRIER_NONE(name, type)         \
    static __always_inline type name(type __x) \
    {                                          \
        return __x;                            \
    }

#ifdef __FLOAT_NOEXCEPT
__OPT_BARRIER_NONE(__nobarrier_float_f, float)
#define __opt_barrier_float_f __nobarrier_float_f
#else
#define __opt_barrier_float_f __barrier_float_f
#endif

#ifdef __DOUBLE_NOEXCEPT
__OPT_BARRIER_NONE(__nobarrier_float_d, double)
#define __opt_barrier_float_d __nobarrier_float_d
#else
#define __opt_barrier_float_d __barrier_float_d
#endif

#ifdef __LONG_DOUBLE_NOEXCEPT
__OPT_BARRIER_NONE(__nobarrier_float_l, long double)
#define __opt_barrier_float_l __nobarrier_float_l
#else
#define __opt_barrier_float_l __barrier_float_l
#endif

#define opt_barrier_float(x)                   \
    _Generic((x),                              \
        float: __opt_barrier_float_f,          \
        double: __opt_barrier_float_d,         \
        long double: __opt_barrier_float_l)(x)

/* Whether the target raises exceptions for each floating point type */
#ifdef __FLOAT_NOEXCEPT
#define __OPT_BARRIER_FLOAT_NOEXCEPT 1
#else
#define __OPT_BARRIER_FLOAT_NOEXCEPT 0
#endif
#ifdef __DOUBLE_NOEXCEPT
#define __OPT_BARRIER_DOUBLE_NOEXCEPT 1
#else
#define __OPT_BARRIER_DOUBLE_NOEXCEPT 0
#endif
#ifdef __LONG_DOUBLE_NOEXCEPT
#define __OPT_BARRIER_LONG_DOUBLE_NOEXCEPT 1
#else
#define __OPT_BARRIER_LONG_DOUBLE_NOEXCEPT 0
#endif

/* An integer holding floating point bits follows the type of its size */
#define __OPT_BARRIER_NOEXCEPT_SIZE(n)                                    \
    ((n) == __SIZEOF_FLOAT__        ? __OPT_BARRIER_FLOAT_NOEXCEPT        \
         : (n) == __SIZEOF_DOUBLE__ ? __OPT_BARRIER_DOUBLE_NOEXCEPT       \
                                    : __OPT_BARRIER_LONG_DOUBLE_NOEXCEPT)

#if __OPT_BARRIER_NOEXCEPT_SIZE(__SIZEOF_INT__)
__OPT_BARRIER_NONE(__nobarrier_int_i, int)
#define __opt_barrier_int_i __nobarrier_int_i
#else
#define __opt_barrier_int_i __barrier_int_i
#endif

#if __OPT_BARRIER_NOEXCEPT_SIZE(__SIZEOF_INT__)
__OPT_BARRIER_NONE(__nobarrier_int_ui, unsigned int)
#define __opt_barrier_int_ui __nobarrier_int_ui
#else
#define __opt_barrier_int_ui __barrier_int_ui
#endif

#if __OPT_BARRIER_NOEXCEPT_SIZE(__SIZEOF_LONG__)
__OPT_BARRIER_NONE(__nobarrier_int_l, long)
#define __opt_barrier_int_l __nobarrier_int_l
#else
#define __opt_barrier_int_l __barrier_int_l
#endif

#if __OPT_BARRIER_NOEXCEPT_SIZE(__SIZEOF_LONG__)
__OPT_BARRIER_NONE(__nobarrier_int_ul, unsigned long)
#define __opt_barrier_int_ul __nobarrier_int_ul
#else
#define __opt_barrier_int_ul __barrier_int_ul
#endif

#if __OPT_BARRIER_NOEXCEPT_SIZE(__SIZEOF_LONG_LONG__)
__OPT_BARRIER_NONE(__nobarrier_int_ll, long long)
#define __opt_barrier_int_ll __nobarrier_int_ll
#else
#define __opt_barrier_int_ll __barrier_int_ll
#endif

#if __OPT_BARRIER_NOEXCEPT_SIZE(__SIZEOF_LONG_LONG__)
__OPT_BARRIER_NONE(__nobarrier_int_ull, unsigned long long)
#define __opt_barrier_int_ull __nobarrier_int_ull
#else
#define __opt_barrier_int_ull __barrier_int_ull
#endif

#define opt_barrier_int(x)                            \
    _Generic((x),                                     \
        int: __opt_barrier_int_i,                     \
        unsigned int: __opt_barrier_int_ui,           \
        long: __opt_barrier_int_l,                    \
        unsigned long: __opt_barrier_int_ul,          \
        long long: __opt_barrier_int_ll,              \
        unsigned long long: __opt_barrier_int_ull)(x)

#ifdef __clang__
#define clang_barrier_float(x) opt_barrier_float(x)
#else
#define clang_barrier_float(x) (x)
#endif

#endif /* __OPT_BARRIER_GENERIC_SELECTION */

#endif /* _OPT_BARRIER_H_ */
