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

#include <stdio.h>

static volatile float x = 0x1.362b4p-127;
static volatile float x2 = 0x1.362b4p-127 * 2;
static volatile float x3 = 0x1.362b4p-127 * 3;

/* compiler-rt gets basic arithmetic wrong on 32-bit arm. */

int
main(void)
{
#ifdef __RX__
    printf("no denorms on rx, skipping\n");
    return 77;
#endif
    int ret = 0;

    /* Fix proposed in https://github.com/llvm/llvm-project/pull/185245 */

    /* subnormal + subnormal = normal */
    if (x + x != x2) {
        printf("x %a x2 %a x + x %a\n", (double)x, (double)x2, (double)(x + x));
        ret = 1;
    }

    /* Extra checks which aren't known to fail */

    /* normal + subnormal */
    if (x2 + x != x3) {
        printf("x %a x2 %a x3 %a x2 + x %a\n", (double)x, (double)x2, (double)x3, (double)(x2 + x));
        ret = 1;
    }

    /* normal - subnormal = subnormal */
    if (x2 - x != x) {
        printf("x %a x2 %a x2 - x %a\n", (double)x, (double)x2, (double)(x2 - x));
        ret = 1;
    }
    return ret;
}
