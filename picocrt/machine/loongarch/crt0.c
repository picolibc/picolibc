/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2024 Jiaxun Yang <<jiaxun.yang@flygoat.com>
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

#include <stddef.h>
#include "../../crt0.h"

static void __used __section(".init")
_cstart(void)
{
	__start();
}

#ifdef CRT0_SEMIHOST
#include <semihost.h>
#include <unistd.h>
#include <stdio.h>

#define NUM_REG 32

#if __loongarch_grlen == 32
#define FMT     "%08lx"
#define SD      "st.d"
#define ADDI    "addi.d"
#else
#define FMT     "%016lx"
#define SD      "st.d"
#define ADDI    "addi.w"
#endif

struct fault {
        unsigned long   r[NUM_REG];
	unsigned long csr_era;
	unsigned long csr_badvaddr;
	unsigned long csr_crmd;
	unsigned long csr_prmd;
	unsigned long csr_euen;
	unsigned long csr_ecfg;
	unsigned long csr_estat;
};

static const char *const names[NUM_REG] = {
        "zero", "ra",   "tp",   "sp",   "a0",   "a1",   "a2",   "a3",
        "a4",   "a5",   "a6",   "a7",   "t0",   "t1",   "t2",   "t3",
        "t4",   "t5",   "t6",   "t7",   "t8",   "r21",  "fp",   "s0",
        "s1",   "s2",   "s3",   "s4",   "s5",   "s6",   "s7",   "s8",
};

static void __used __section(".init")
_ctrap(struct fault *fault)
{
        int r;
        printf("LoongArch fault\n");
        for (r = 0; r < NUM_REG; r++)
                printf("\tx%d %-5.5s%s 0x" FMT "\n", r, names[r], r < 10 ? " " : "", fault->r[r]);
        printf("\tera: 0x%lx\n", fault->csr_era);
        printf("\tbadvaddr: 0x%lx\n", fault->csr_badvaddr);
        printf("\tcrmd: 0x%lx\n", fault->csr_crmd);
        printf("\tprmd: 0x%lx\n", fault->csr_prmd);
        printf("\teuen: 0x%lx\n", fault->csr_euen);
        printf("\tecfg: 0x%lx\n", fault->csr_ecfg);
        printf("\testat: 0x%lx\n", fault->csr_estat);
        _exit(1);
}

#define _PASTE(r) #r
#define PASTE(r) _PASTE(r)

void  __section(".init") __used __attribute((aligned(0x40)))
_trap(void)
{
        /* Build a known-working C environment */
	__asm__(
                "csrwr          $sp, 0x30\n" /* Save sp to ksave0 */
                "la.abs	        $sp, __heap_end\n");

        /* Make space for saved registers */
        __asm__(ADDI "          $sp, $sp, %0\n"
                ".cfi_def_cfa   3, 0\n" // sp
                :: "i"(-sizeof(struct fault)));

        /* Save registers on stack */
#define SAVE_REG(num)   \
        __asm__(SD"     $r%0, $sp, %1" :: "i" (num), \
                "i" ((num) * sizeof(unsigned long) + offsetof(struct fault, r)))

        SAVE_REG(0);
        SAVE_REG(1);
        SAVE_REG(2);
        SAVE_REG(3);
        SAVE_REG(4);
        SAVE_REG(5);
        SAVE_REG(6);
        SAVE_REG(7);
        SAVE_REG(8);
        SAVE_REG(9);
        SAVE_REG(10);
        SAVE_REG(11);
        SAVE_REG(12);
        SAVE_REG(13);
        SAVE_REG(14);
        SAVE_REG(15);
        SAVE_REG(16);
        SAVE_REG(17);
        SAVE_REG(18);
        SAVE_REG(19);
        SAVE_REG(20);
        SAVE_REG(21);
        SAVE_REG(22);
        SAVE_REG(23);
        SAVE_REG(24);
        SAVE_REG(25);
        SAVE_REG(26);
        SAVE_REG(27);
        SAVE_REG(28);
        SAVE_REG(29);
        SAVE_REG(30);
        SAVE_REG(31);

#define SAVE_CSR(name, reg)  \
        __asm__("csrrd   $t0, "PASTE(reg)); \
        __asm__(SD"      $t0, $sp, %0" :: "i" (offsetof(struct fault, name)))


        /*
         * Save the trapping frame's stack pointer that was stashed in ksave0
         * and tell the unwinder where we can find the return address (csr_era).
         */
        __asm__("csrrd   $ra, 0x6\n"
                SD "     $ra, $sp, %0\n"
                ".cfi_offset 1, %0\n" // ra
                "csrrd   $t0, 0x30\n"
                SD "     $t0, $sp, %0\n"
                ".cfi_offset 3, %1\n" // sp
                :: "i"(offsetof(struct fault, csr_era)),
                   "i"(offsetof(struct fault, r[3])));
        SAVE_CSR(csr_badvaddr, 0x7);
        SAVE_CSR(csr_crmd, 0x0);
        SAVE_CSR(csr_prmd, 0x1);
        SAVE_CSR(csr_euen, 0x2);
        SAVE_CSR(csr_ecfg, 0x4);
        SAVE_CSR(csr_estat, 0x5);

        /*
         * Pass pointer to saved registers in first parameter register
         */
        __asm__("move     $a0, $sp");

        /* Enable FPU (just in case) */
#ifndef __loongarch_soft_float
	__asm__("li.w           $t0, 0x1\n"        /* FPEN */
                "csrxchg	$t0, $t0, 0x2\n"); /* EUEN */
#endif
        __asm__("la.pcrel       $t0, _ctrap\n"
                "jirl		$ra, $t0, 0\n");
}
#endif

void __section(".text.init.enter") __used
_start(void)
{
	__asm__("la.abs         $sp, __stack\n");

#ifndef __loongarch_soft_float
	__asm__("li.w           $t0, 0x1\n"        /* FPEN */
                "csrxchg	$t0, $t0, 0x2\n"); /* EUEN */
#endif

#ifdef CRT0_SEMIHOST
        __asm__("la.abs         $t0, _trap");
        __asm__("csrwr          $t0, 0xc");
#endif
        __asm__("la.pcrel      $t0, _cstart\n"
                "jirl          $ra, $t0, 0\n");
}
