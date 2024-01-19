/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Sebastian Meyer
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

static void __attribute__((used)) __section(".init")
_cstart(void)
{
	__start();
}

#ifdef CRT0_SEMIHOST
#include <semihost.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __riscv_32e
#define NUM_REG 16
#else
#define NUM_REG 32
#endif

#if __riscv_xlen == 32
#define FMT     "%08lx"
#define SD      "sw"
#else
#define FMT     "%016lx"
#define SD      "sd"
#endif

struct fault {
        unsigned long   r[NUM_REG];
        unsigned long   mepc;
        unsigned long   mcause;
        unsigned long   mtval;
};

static const char *const names[NUM_REG] = {
        "zero", "ra",   "sp",   "gp",   "tp",   "t0",   "t1",   "t2",
        "s0/fp","s1",   "a0",   "a1",   "a2",   "a3",   "a4",   "a5",
#if NUM_REG > 16
        "a6",   "a7",   "s2",   "s3",   "s4",   "s5",   "s6",   "s7",
        "s8",   "s9",   "s10",  "s11",  "t3",   "t4",   "t5",   "t6",
#endif
};


static void __attribute__((used)) __section(".init")
_ctrap(struct fault *fault)
{
        int r;
        printf("RISCV fault\n");
        for (r = 0; r < NUM_REG; r++)
                printf("\tx%d %-5.5s%s 0x" FMT "\n", r, names[r], r < 10 ? " " : "", fault->r[r]);
        printf("\tmepc:     0x" FMT "\n", fault->mepc);
        printf("\tmcause:   0x" FMT "\n", fault->mcause);
        printf("\tmtval:    0x" FMT "\n", fault->mtval);
        _exit(1);
}

#define _PASTE(r) #r
#define PASTE(r) _PASTE(r)

void __attribute__((naked)) __section(".init") __attribute__((used)) __attribute((aligned(4)))
_trap(void)
{
#ifndef __clang__
        __asm__(".option	nopic");
#endif

        /* Build a known-working C environment */
	__asm__(".option	push\n"
                ".option	norelax\n"
                "csrrw  sp, mscratch, sp\n"
                "la	sp, __heap_end\n"
                ".option	pop");

        /* Make space for saved registers */
        __asm__("addi   sp, sp, %0\n"
                ".cfi_def_cfa sp, 0\n"
                :: "i"(-sizeof(struct fault)));

        /* Save registers on stack */
#define SAVE_REG(num)   \
        __asm__(SD"     x%0, %1(sp)" :: "i" (num), \
                "i" ((num) * sizeof(unsigned long) + offsetof(struct fault, r)))

#define SAVE_REGS_8(base) \
        SAVE_REG(base+0); SAVE_REG(base+1); SAVE_REG(base+2); SAVE_REG(base+3); \
        SAVE_REG(base+4); SAVE_REG(base+5); SAVE_REG(base+6); SAVE_REG(base+7)

        SAVE_REGS_8(0);
        SAVE_REGS_8(8);
#ifndef __riscv_32e
        SAVE_REGS_8(16);
        SAVE_REGS_8(24);
#endif

#define SAVE_CSR(name)  \
        __asm__("csrr   t0, "PASTE(name));\
        __asm__(SD"  t0, %0(sp)" :: "i" (offsetof(struct fault, name)))

        /*
         * Save the trapping frame's stack pointer that was stashed in mscratch
         * and tell the unwinder where we can find the return address (mepc).
         */
        __asm__("csrr   ra, mepc\n"
                SD "    ra, %0(sp)\n"
                ".cfi_offset ra, %0\n"
                "csrrw t0, mscratch, zero\n"
                SD "    t0, %1(sp)\n"
                ".cfi_offset sp, %1\n"
                :: "i"(offsetof(struct fault, mepc)),
                   "i"(offsetof(struct fault, r[2])));
        SAVE_CSR(mcause);
        SAVE_CSR(mtval);

        /*
         * Pass pointer to saved registers in first parameter register
         */
        __asm__("la	gp, __global_pointer$");
        __asm__("mv     a0, sp");

        /* Enable FPU (just in case) */
#ifdef __riscv_flen
	__asm__("csrr	t0, mstatus\n"
                "li	t1, 8192\n"     	// 1 << 13 = 8192
                "or	t0, t1, t0\n"
                "csrw	mstatus, t0\n"
                "csrwi	fcsr, 0");
#endif
        __asm__("jal    _ctrap");
}
#endif

void __attribute__((naked)) __section(".text.init.enter") __attribute__((used))
_start(void)
{

	/**
	 * seems clang has no option "nopic". Now this could be problematic,
	 * since according to the clang devs at [0], that option has an effect
	 * on `la`. However, the resulting crt0.o looks the same as the one from
	 * gcc (same opcodes + pc relative relocations where I used `la`), so
	 * this could be okay.
	 * [0] https://reviews.llvm.org/D55325
	 */
#ifndef __clang__
        __asm__(".option	nopic");
#endif

	__asm__(".option	push\n"
                ".option	norelax\n"
                "la	sp, __stack\n"
                "la	gp, __global_pointer$\n"
                ".option	pop");

#ifdef __riscv_flen
	__asm__("csrr	t0, mstatus\n"
                "li	t1, 8192\n"     	// 1 << 13 = 8192
                "or	t0, t1, t0\n"
                "csrw	mstatus, t0\n"
                "csrwi	fcsr, 0");
#endif
#ifdef CRT0_SEMIHOST
        __asm__("la     t0, _trap");
        __asm__("csrw   mtvec, t0");
        __asm__("csrr   t1, mtvec");
#endif
        __asm__("j      _cstart");
}
