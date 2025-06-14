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

#define WANT_FLOAT80

#define MAX_ARG 1756

/* 29 coefficients */
#define COEFFS {                                \
        float_80(0x1p0),                        \
        float_80(-0x1.2788cfc6fb618f46p-1),     \
        float_80(0x1.fa658c23b1578254p-1),      \
        float_80(-0x1.d0a118f324b3aa3ep-1),     \
        float_80(0x1.f6a5105508c989f4p-1),      \
        float_80(-0x1.f6c80ec372ce897ep-1),     \
        float_80(0x1.fc7e0a6c32c59d56p-1),      \
        float_80(-0x1.fdf3f12950725b54p-1),     \
        float_80(0x1.ff07b31fff79de44p-1),      \
        float_80(-0x1.ff8022bd1f515118p-1),     \
        float_80(0x1.ffbfa41b1eae45a4p-1),      \
        float_80(-0x1.ffda2eee8fc360cp-1),      \
        float_80(0x1.ffcf64bfbcc0d79ep-1),      \
        float_80(-0x1.ff62de106d363db4p-1),     \
        float_80(0x1.fdc29891cef8e1dp-1),       \
        float_80(-0x1.f8d549228ab04c22p-1),     \
        float_80(0x1.ec6c411783741f9ap-1),      \
        float_80(-0x1.d238fe9f01c49214p-1),     \
        float_80(0x1.a3c917d73dca8e5p-1),       \
        float_80(-0x1.5ebf72442f96e85ep-1),     \
        float_80(0x1.08e8df67ab7c87d4p-1),      \
        float_80(-0x1.603ec88e323cd66p-2),      \
        float_80(0x1.915e6f90031df2fap-3),      \
        float_80(-0x1.7cd05e51298c535ep-4),     \
        float_80(0x1.22dd143aa682d30ep-5),      \
        float_80(-0x1.56219a3fed3533aap-7),     \
        float_80(0x1.21deb84e1a3414f6p-9),      \
        float_80(-0x1.3a1b9187f3ac4048p-12),    \
        float_80(0x1.465696f13cd90dfap-16),     \
    }

#include "gamma_inc.h"
