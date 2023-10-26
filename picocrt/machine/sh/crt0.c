/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

#include "../../crt0.h"

static void __attribute__((used)) __section(".text.startup")
_cstart(void)
{
    __start();
}

extern char __stack[];

#define FPSCR_FR        (1 << 21)
#define FPSCR_SZ        (1 << 20)
#define FPSCR_PR        (1 << 19)
#define FPSCR_DN        (1 << 18)
#define FPSCR_RN        (0 << 0)
#define FPSCR_RM        (3 << 0)

void __section(".init") __attribute__((used))
_start(void)
{
    /* Set up the stack pointer */
    __asm__("mov %0, r15" : : "r" (__stack));

    /* Initialize the FPU */
#ifdef __SH_FPU_ANY__
    long fpscr;
    __asm__("sts fpscr,%0" : "=r" (fpscr));
    /* enable denorms */
    fpscr &= ~FPSCR_DN;
    /* set round to nearest */
    fpscr &= ~FPSCR_RM;
    fpscr |= FPSCR_RN;
#ifdef __SH4__
    fpscr |= FPSCR_PR;
#elif defined(__SH4_SINGLE_ONLY__)
    fpscr &= ~FPSCR_PR;
#endif
    __asm__("lds %0,fpscr" : : "r" (fpscr));
#endif
    /* Branch to C code */
    __asm__("jmp @%0" : : "r" (_cstart));

    /* Fill in any delay slot */
    __asm__("nop");
}
