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

#define _ISOC23_SOURCE
#include "math_two.h"

#ifdef WANT_FLOAT32

#define GET_EXP(_ix, _x)         \
    do {                         \
        GET_FLOAT_WORD(_ix, _x); \
        (_ix) &= 0x7fffffff;     \
    } while (0)

#define EXP_IS_NONFINITE(_ix) (!FLT_UWORD_IS_FINITE(_ix))

#define EXP_ONE_QUARTER       0x3e800000

#endif

#ifdef WANT_FLOAT64

#define GET_EXP(_ix, _x)        \
    do {                        \
        GET_HIGH_WORD(_ix, _x); \
        (_ix) &= 0x7fffffff;    \
    } while (0)

#define EXP_IS_NONFINITE(_ix) ((_ix) >= 0x7ff00000)

#define EXP_ONE_QUARTER       0x3fd00000

#endif

#ifdef WANT_FLOAT80

#define GET_EXP(_ix, _x)                    \
    do {                                    \
        u_int32_t se, i0, i1;               \
                                            \
        GET_LDOUBLE_WORDS(se, i0, i1, x);   \
        (void)i1;                           \
        (_ix) = se & 0x7fff;                \
        (_ix) = ((_ix) << 16) | (i0 >> 16); \
    } while (0)

#define EXP_IS_NONFINITE(_ix) ((_ix) >= 0x7fff0000)

#define EXP_ONE_QUARTER       0x3fe80000

#endif

#ifdef WANT_FLOAT128

#define GET_EXP(_ix, _x)          \
    do {                          \
        GET_LDOUBLE_EXP(_ix, _x); \
        (_ix) &= 0x7fff;          \
    } while (0)

#define EXP_IS_NONFINITE(_ix) ((_ix) >= 0x7fff)

#define EXP_ONE_QUARTER       0x3ff8

#endif
