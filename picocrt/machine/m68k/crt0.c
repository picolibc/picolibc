/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

extern char __stack[];

void        __section(".init") __used _start(void)
{
    /* Generate a reference to __exception_vector so we get one loaded */
    __asm__(".equ __my_exception_vector, __exception_vector");

    /* Initialize stack pointer */
    __asm__("lea %0,%%sp" : : "m"(__stack));

#if __HAVE_68881__
    /* Enable floating point exceptions, set precision and rounding mode */

    __asm__("fmove.l %0,%%fpcr" : : "i"(0));
    __asm__("fmove.l %0,%%fpsr" : : "i"(0));
#endif

    /* Branch to C code */
    __start();
}

#ifdef CRT0_SEMIHOST

/*
 * Trap faults, print message and exit when running under semihost
 */

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

struct fault {
    uint32_t d[8];
    uint32_t a[8];
    uint16_t ccr;   /* pushed by processor */
    uint16_t pc_hi; /* pushed by processor */
    uint16_t pc_lo; /* pushed by processor */
};

void
m68k_fault(struct fault *fault, char *reason)
{
    size_t i;
    printf("M68k fault: %s\n", reason);
    for (i = 0; i < 8; i++)
        printf("D%ld: 0x%08lx\n", i, fault->d[i]);
    for (i = 0; i < 8; i++)
        printf("A%ld: 0x%08lx\n", i, fault->a[i]);
    printf("PC: 0x%08lx\n", ((uint32_t)fault->pc_hi << 16) | (uint32_t)fault->pc_lo);
    printf("SR: 0x%04x\n", fault->ccr);
    _exit(1);
}

#if 0
#define ISR_COMMON                 \
    struct fault *sp;              \
    __asm__("move.l %%a7,-(%%sp);" \
            "move.l %%a6,-(%%sp);" \
            "move.l %%a5,-(%%sp);" \
            "move.l %%a4,-(%%sp);" \
            "move.l %%a3,-(%%sp);" \
            "move.l %%a2,-(%%sp);" \
            "move.l %%a1,-(%%sp);" \
            "move.l %%a0,-(%%sp);" \
            "move.l %%d7,-(%%sp);" \
            "move.l %%d6,-(%%sp);" \
            "move.l %%d5,-(%%sp);" \
            "move.l %%d4,-(%%sp);" \
            "move.l %%d3,-(%%sp);" \
            "move.l %%d2,-(%%sp);" \
            "move.l %%d1,-(%%sp);" \
            "move.l %%d0,-(%%sp);" \
            "move.l %%sp,%0"       \
            : "=d"(sp));
#endif
#define ISR_COMMON                    \
    struct fault *sp;                 \
    __asm__("move.l %%a7,-(%%sp);"    \
            "suba.l #60,%%sp;"        \
            "movem.l #0x7fff,(%%sp);" \
            "move.l %%sp,%0"          \
            : "=d"(sp));

#define _isr_name(name) m68k_##name##_isr
#define isr_name(name)  _isr_name(name)
#define isr(name)              \
    void isr_name(name)(void)  \
    {                          \
        ISR_COMMON;            \
        m68k_fault(sp, #name); \
    }

isr(access_fault) isr(address_error) isr(illegal_instruction) isr(divide_by_zero) isr(chk) isr(trap)

    isr(fp_branch_uc) isr(fp_inexact) isr(fp_divide_by_zero) isr(fp_underflow)

        isr(fp_operand_error) isr(fp_overflow) isr(fp_signaling_nan) isr(fp_unimplemented_data)

#endif
