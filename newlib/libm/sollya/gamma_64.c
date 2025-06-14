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

#define WANT_FLOAT64

#define MAX_ARG 172

/* 23 coefficients */
#define COEFFS {                                \
        float_64(0x1p0),                        \
        float_64(-0x1.2788cfc6fb5ffp-1),        \
        float_64(0x1.fa658c23aff89p-1),         \
        float_64(-0x1.d0a118f2bb037p-1),        \
        float_64(0x1.f6a51044a2817p-1),         \
        float_64(-0x1.f6c80d37b4ea2p-1),        \
        float_64(0x1.fc7df183d8813p-1),         \
        float_64(-0x1.fdf2d44dced38p-1),        \
        float_64(0x1.fefe46762c689p-1),         \
        float_64(-0x1.ff432ab0c2d09p-1),        \
        float_64(0x1.fe8b500b54b7cp-1),         \
        float_64(-0x1.fb01539db819ap-1),        \
        float_64(0x1.f0112ce9666d3p-1),         \
        float_64(-0x1.d5a6922e2813fp-1),        \
        float_64(0x1.a2988f2a1162ap-1),         \
        float_64(-0x1.53678b6fe269ap-1),        \
        float_64(0x1.e24bd24cef707p-2),         \
        float_64(-0x1.20d8e6dd2d567p-2),        \
        float_64(0x1.17ba283475de8p-3),         \
        float_64(-0x1.a0ac3b5e68e8bp-5),        \
        float_64(0x1.bceb7f6a307bp-7),          \
        float_64(-0x1.2db744923dd1dp-9),        \
        float_64(0x1.852e26d23416p-13),         \
    }

#include "gamma_inc.h"

