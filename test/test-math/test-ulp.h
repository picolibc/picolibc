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

#include <sys/types.h>

math_ulps_t math_ulps[] = {
#ifdef __PICOLIBC__

    /* Trig functions */
    { .name = "acos",       .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "acosh",      .b32 =       1, .b64 =       1, .b80 =       2, .b128 =       1 },
    { .name = "asin",       .b32 =       0, .b64 =       0, .b80 =       0, .b128 =       1 },
    { .name = "asinh",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "atan",       .b32 =       0, .b64 =       0, .b80 =       0, .b128 =       1 },
    { .name = "atan2",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "atanh",      .b32 =       1, .b64 =       1, .b80 =       7, .b128 =       1 },
    { .name = "cos",        .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "cosh",       .b32 =       1, .b64 =       0, .b80 =       1, .b128 =       1 },
    { .name = "sin",        .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "sinh",       .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "tan",        .b32 =       0, .b64 =       1, .b80 =       0, .b128 =       1 },
    { .name = "tanh",       .b32 =       0, .b64 =       1, .b80 =       1, .b128 =       1 },

    /* Exp/log functions */
    { .name = "exp",        .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "exp2",       .b32 =       1, .b64 =       1, .b80 =       2, .b128 =       0 },
    { .name = "expm1",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "log10",      .b32 =       1, .b64 =       0, .b80 =       1, .b128 =       1 },
    { .name = "log1p",      .b32 =       1, .b64 =       1, .b80 =       2, .b128 =       1 },
    { .name = "log2",       .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "log",        .b32 =       1, .b64 =       0, .b80 =       0, .b128 =       1 },
    { .name = "pow",        .b32 =       1, .b64 =       1, .b80 = MAX_ULP, .b128 =       1 },

    /* Misc functions */
    { .name = "cbrt",       .b32 =       2, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "erf",        .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
    { .name = "erfc",       .b32 =       1, .b64 =       0, .b80 =       0, .b128 =       1 },
    { .name = "hypot",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "j0",         .b32 =       2, .b64 =       1, .b80 =       0, .b128 =       0 },
#else
    { .name = "j0",         .b32 =       1, .b64 =       1, .b80 =       0, .b128 =       0 },
#endif
    { .name = "j1",         .b32 =       3, .b64 =       1, .b80 =       0, .b128 =       0 },
    { .name = "jn",         .b32 =       3, .b64 =       3, .b80 =       0, .b128 =       0 },
    { .name = "lgamma",     .b32 = MAX_ULP, .b64 =       2, .b80 =       2, .b128 =       1 },
#if defined(__riscv_float_abi_soft) || defined(__clang__)
    { .name = "sqrt",       .b32 =       0, .b64 =       0, .b80 =       0, .b128 =       1 },
#elif defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "sqrt",       .b32 =       1, .b64 =       1, .b80 =       0, .b128 =       0 },
#else
    { .name = "sqrt",       .b32 =       0, .b64 =       0, .b80 =       0, .b128 =       0 },
#endif
    { .name = "tgamma",     .b32 = MAX_ULP, .b64 =      56, .b80 =       2, .b128 =      97 },

    /* Complex trig functions */
#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "cacos",      .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 =       2, .b128 = MAX_ULP },
#else
    { .name = "cacos",      .b32 =       2, .b64 =       3, .b80 =       2, .b128 = MAX_ULP },
#endif
#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "cacosh",     .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 =       2, .b128 = MAX_ULP },
#else
    { .name = "cacosh",     .b32 =       1, .b64 =       2, .b80 =       2, .b128 = MAX_ULP },
#endif
    { .name = "casin",      .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 = MAX_ULP, .b128 = MAX_ULP },
    { .name = "casinh",     .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 = MAX_ULP, .b128 = MAX_ULP },
    { .name = "catan",      .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 = MAX_ULP, .b128 = MAX_ULP },
    { .name = "catanh",     .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 = MAX_ULP, .b128 = MAX_ULP },
    { .name = "ccos",       .b32 = INV_ULP, .b64 = INV_ULP, .b80 = INV_ULP, .b128 = INV_ULP },
#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "ccosh",      .b32 = INV_ULP, .b64 = MAX_ULP, .b80 =       1, .b128 =       2 },
#else
    { .name = "ccosh",      .b32 = INV_ULP, .b64 =       1, .b80 =       1, .b128 =       2 },
#endif
    { .name = "csin",       .b32 = INV_ULP, .b64 = INV_ULP, .b80 = INV_ULP, .b128 = INV_ULP },
    { .name = "csinh",      .b32 = INV_ULP, .b64 = INV_ULP, .b80 = INV_ULP, .b128 = INV_ULP },

#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "ctan",       .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 =       3, .b128 =       1 },
#elif defined(__clang__) && defined(__SOFTFP__)
    { .name = "ctan",       .b32 = MAX_ULP, .b64 =       2, .b80 =       3, .b128 =       1 },
#else
    { .name = "ctan",       .b32 =       1, .b64 =       2, .b80 =       3, .b128 =       1 },
#endif
#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "ctanh",      .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 =       3, .b128 =       1 },
#elif defined(__clang__) && defined(__SOFTFP__)
    { .name = "ctanh",      .b32 = MAX_ULP, .b64 =       2, .b80 =       3, .b128 =       1 },
#else
    { .name = "ctanh",      .b32 =       1, .b64 =       2, .b80 =       3, .b128 =       1 },
#endif

    /* Complex exp/log functions */
    { .name = "cexp",       .b32 = INV_ULP, .b64 = INV_ULP, .b80 = INV_ULP, .b128 = INV_ULP },
#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "clog",       .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 =       5, .b128 =       6 },
#elif defined(__riscv_float_abi_soft) || defined(__clang__)
    { .name = "clog",       .b32 =       4, .b64 =       6, .b80 =       5, .b128 =       8 },
#else
    { .name = "clog",       .b32 =       4, .b64 =       6, .b80 =       5, .b128 =       6 },
#endif
    { .name = "cpow",       .b32 = INV_ULP, .b64 = INV_ULP, .b80 = INV_ULP, .b128 = INV_ULP },

    /* Complex misc functions */
#if defined(__m68k__) && FLT_EVAL_METHOD != 0
    { .name = "csqrt",      .b32 = MAX_ULP, .b64 = MAX_ULP, .b80 =       1, .b128 =       1 },
#elif defined(__riscv_float_abi_soft) || defined(__clang__)
    { .name = "csqrt",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       2 },
#else
    { .name = "csqrt",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       1 },
#endif

#else   /* ! __PICOLIBC__ */

    /* Trig functions */
    { .name = "acos",       .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "acosh",      .b32 =       0, .b64 =       1, .b80 =       2, .b128 =       0 },
    { .name = "asin",       .b32 =       0, .b64 =       0, .b80 =       0, .b128 =       0 },
    { .name = "asinh",      .b32 =       0, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "atan",       .b32 =       0, .b64 =       0, .b80 =       0, .b128 =       0 },
    { .name = "atan2",      .b32 =       1, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "atanh",      .b32 =       0, .b64 =       1, .b80 =       2, .b128 =       0 },
    { .name = "cos",        .b32 =       1, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "cosh",       .b32 =       0, .b64 =       0, .b80 =       0, .b128 =       0 },
    { .name = "sin",        .b32 =       1, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "sinh",       .b32 =       0, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "tan",        .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "tanh",       .b32 =       0, .b64 =       1, .b80 =       1, .b128 =       0 },

    /* Exp/log functions */
    { .name = "exp",        .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "exp2",       .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "expm1",      .b32 =       0, .b64 =       1, .b80 =       2, .b128 =       0 },
    { .name = "log",        .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "log10",      .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "log1p",      .b32 =       0, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "log2",       .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "pow",        .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },

    /* Misc functions */
    { .name = "cbrt",       .b32 =       0, .b64 =       2, .b80 =       1, .b128 =       0 },
    { .name = "erf",        .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "erfc",       .b32 =       0, .b64 =       0, .b80 =       1, .b128 =       0 },
    { .name = "hypot",      .b32 =       0, .b64 =       1, .b80 =       0, .b128 =       0 },
    { .name = "j0",         .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "j1",         .b32 =       2, .b64 =       1, .b80 =       3, .b128 =       0 },
    { .name = "jn",         .b32 =       2, .b64 =       2, .b80 =       3, .b128 =       0 },
    { .name = "lgamma",     .b32 =       0, .b64 =       2, .b80 =       1, .b128 =       0 },
    { .name = "sqrt",       .b32 =       1, .b64 =       0, .b80 =       0, .b128 =       0 },
    { .name = "tgamma",     .b32 =       1, .b64 =       2, .b80 =       2, .b128 =       0 },

    /* Complex trig functions */
    { .name = "cacos",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "cacosh",     .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "casin",      .b32 =       2, .b64 =       2, .b80 =       2, .b128 =       0 },
    { .name = "casinh",     .b32 =       2, .b64 =       3, .b80 =       2, .b128 =       0 },
    { .name = "catan",      .b32 =       2, .b64 =       2, .b80 =       2, .b128 =       0 },
    { .name = "catanh",     .b32 =       2, .b64 =       2, .b80 =       2, .b128 =       0 },
    { .name = "ccos",       .b32 =       2, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "ccosh",      .b32 =       2, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "csin",       .b32 =       1, .b64 =       1, .b80 =       2, .b128 =       0 },
    { .name = "csinh",      .b32 =       1, .b64 =       1, .b80 =       2, .b128 =       0 },
    { .name = "ctan",       .b32 =       1, .b64 =       2, .b80 =       3, .b128 =       0 },
    { .name = "ctanh",      .b32 =       1, .b64 =       2, .b80 =       3, .b128 =       0 },

    /* Complex exp/log functions */
    { .name = "cexp",       .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "clog",       .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },
    { .name = "cpow",       .b32 =     169, .b64 = MAX_ULP, .b80 = MAX_ULP, .b128 =       0 },

    /* Complex misc functions */
    { .name = "csqrt",      .b32 =       1, .b64 =       1, .b80 =       1, .b128 =       0 },

#endif
};
