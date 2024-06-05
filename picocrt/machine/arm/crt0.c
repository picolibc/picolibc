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

#include <picolibc.h>
#include "../../crt0.h"

#if __ARM_ARCH_PROFILE == 'M'

/*
 * Cortex-mM includes an NVIC and starts with SP initialized, so start
 * is a C function
 */

extern const void *__interrupt_vector[];

#define CPACR	((volatile uint32_t *) (0xE000ED88))

#ifdef __clang__
const void *__interrupt_reference = __interrupt_vector;
#endif

void
_start(void)
{
	/* Generate a reference to __interrupt_vector so we get one loaded */
	__asm__(".equ __my_interrupt_vector, __interrupt_vector");
    /* Access to the coprocessor has to be enabled in CPACR, if either FPU or
     * MVE is used. This is described in "Arm v8-M Architecture Reference
     * Manual". */
#if defined __ARM_FP || defined __ARM_FEATURE_MVE
	/* Enable FPU */
	*CPACR |= 0xf << 20;
	/*
	 * Wait for the write enabling FPU to reach memory before
	 * executing the instruction accessing the status register
	 */
	__asm__("dsb");
	__asm__("isb");

        /* Clear FPU status register. 0x40000 will initialize FPSCR.LTPSIZE to
         * a valid value for 8.1-m low overhead loops. */
#if __ARM_ARCH >= 8 && __ARM_ARCH_PROFILE == 'M'
#define INIT_FPSCR 0x40000
#else
#define INIT_FPSCR 0x0
#endif
	__asm__("vmsr fpscr, %0" : : "r" (INIT_FPSCR));
#endif

#if defined(__ARM_FEATURE_PAUTH) || defined(__ARM_FEATURE_BTI)
        uint32_t        control;
        __asm__("mrs %0, CONTROL" : "=r" (control));
#ifdef __ARM_FEATURE_PAUTH
        control |= (3 << 6);
#endif
#ifdef __ARM_FEATURE_BTI
        control |= (3 << 4);
#endif
        __asm__("msr CONTROL, %0" : : "r" (control));
#endif
	__start();
}

#else

/*
 * Regular ARM has an 8-entry exception vector and starts without SP
 * initialized, so start is a naked function
 */

static void __attribute__((used)) __section(".init")
_cstart(void)
{
	__start();
}

extern char __stack[];

#ifdef PICOCRT_ENABLE_CACHES
/* We need 4096 1MB mappings to cover the full 32-bit address space. */
#define MMU_MAPPING_COUNT 4096
extern uint32_t identity_page_table[MMU_MAPPING_COUNT];
#define MMU_TYPE_1MB (0x2 << 0)
#define MMU_RW (0x3 << 10)
#define MMU_NORMAL_CACHEABLE ((0x0 << 12) | (0x3 << 2))
#define MMU_MAPPING_FLAGS (MMU_TYPE_1MB | MMU_RW | MMU_NORMAL_CACHEABLE)
__asm__(
    ".rodata\n"
    ".global identity_page_table\n"
    ".balign 16384\n"
    "identity_page_table:\n"
    ".set _i, 0\n"
    ".rept " __XSTRING(MMU_MAPPING_COUNT) "\n"
    "  .4byte (_i << 20) |" __XSTRING(MMU_MAPPING_FLAGS) "\n"
    "  .set _i, _i + 1\n"
    ".endr\n"
    ".size identity_page_table, " __XSTRING(MMU_MAPPING_COUNT * 4) "\n"
);
#endif

void __attribute__((naked)) __section(".init") __attribute__((used))
_start(void)
{
	/* Generate a reference to __vector_table so we get one loaded */
	__asm__(".equ __my_vector_table, __vector_table");

	/* Initialize stack pointer */
	__asm__("mov sp, %0" : : "r" (__stack));

#ifdef __thumb2__
	/* Make exceptions run in Thumb mode */
	uint32_t sctlr;
	__asm__("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
	sctlr |= (1 << 30);
	__asm__("mcr p15, 0, %0, c1, c0, 0" : : "r" (sctlr));
#endif
#if defined __ARM_FP || defined __ARM_FEATURE_MVE
#if __ARM_ARCH > 6
	/* Set CPACR for access to CP10 and 11 */
	__asm__("mcr p15, 0, %0, c1, c0, 2" : : "r" (0xf << 20));
#endif
	/* Enable FPU */
	__asm__("vmsr fpexc, %0" : : "r" (0x40000000));
#endif

#ifdef PICOCRT_ENABLE_CACHES
#define SCTLR_MMU (1 << 0)
#define SCTLR_DATA_L2 (1 << 2)
#define SCTLR_BRANCH_PRED (1 << 11)
#define SCTLR_ICACHE (1 << 12)
    /* We have to set up an identity map and enable the MMU for caches.
     * Additionally, all page table entries are set to Domain 0, so set up DACR
     * so that Domain zero has permission checks enabled rather than "deny all".
     */
    __asm__(
        "mov r0, #1\n"
        "mcr p15, 0, r0, c3, c0, 0\n" /* Set DACR Domain 0 permissions checked */
        "mcr p15, 0, %0, c2, c0, 0\n" /* Write TTBR */
        /*
         * No DSB since tables are statically initialized and dcache is off.
         * We or identity_page_table with 0x3 to set the cacheable flag bits.
         */
        :: "r" ((uintptr_t)identity_page_table | 0x3) : "r0");
    /* Note: we assume Data+L2 cache has been invalidated by reset. */
    __asm__(
        "mov r0, #0\n"
        "mcr p15, 0, r0, c7, c5, 0\n" /* ICIALLU: invalidate instruction cache */
        "mcr p15, 0, r0, c8, c7, 0\n" /* TLBIALL: invalidate TLB */
        "mcr p15, 0, r0, c7, c5, 6\n" /* BPIALL: invalidate branch predictor */
        "isb\n"
        "mrc p15, 0, r0, c1, c0, 0\n" /* Read SCTLR */
        /* Need to split this into two instructions due to immediate range. */
        "orr r0, r0, #" __XSTRING(SCTLR_ICACHE | SCTLR_BRANCH_PRED) " \n"
        "orr r0, r0, #" __XSTRING(SCTLR_DATA_L2 | SCTLR_MMU) " \n"
        "mcr p15, 0, r0, c1, c0, 0\n" /* Write back SCTLR */
        "isb\n"
    );
#endif  /* PICOCRT_ENABLE_CACHES */

	/* Branch to C code */
	__asm__("b _cstart");
}

#endif

#ifdef CRT0_SEMIHOST

/*
 * Trap faults, print message and exit when running under semihost
 */

#include <semihost.h>
#include <unistd.h>
#include <stdio.h>

#define _REASON(r) #r
#define REASON(r) _REASON(r)

static void arm_fault_write_reg(const char *prefix, unsigned reg)
{
    fputs(prefix, stdout);

    for (unsigned i = 0; i < 8; i++) {
        unsigned digitval = 0xF & (reg >> (28 - 4*i));
        char digitchr = '0' + digitval + (digitval >= 10 ? 'a'-'0'-10 : 0);
        putchar(digitchr);
    }

    putchar('\n');
}

#if __ARM_ARCH_PROFILE == 'M'

#define GET_SP  struct fault *sp; __asm__ ("mov %0, sp" : "=r" (sp))

struct fault {
    unsigned int        r0;
    unsigned int        r1;
    unsigned int        r2;
    unsigned int        r3;
    unsigned int        r12;
    unsigned int        lr;
    unsigned int        pc;
    unsigned int        xpsr;
};

static const char *const reasons[] = {
    "hardfault\n",
    "memmanage\n",
    "busfault\n",
    "usagefault\n"
};

#define REASON_HARDFAULT        0
#define REASON_MEMMANAGE        1
#define REASON_BUSFAULT         2
#define REASON_USAGE            3

static void __attribute__((used))
arm_fault(struct fault *f, int reason)
{
    fputs("ARM fault: ", stdout);
    fputs(reasons[reason], stdout);
    arm_fault_write_reg("\tR0:   0x", f->r0);
    arm_fault_write_reg("\tR1:   0x", f->r1);
    arm_fault_write_reg("\tR2:   0x", f->r2);
    arm_fault_write_reg("\tR3:   0x", f->r3);
    arm_fault_write_reg("\tR12:  0x", f->r12);
    arm_fault_write_reg("\tLR:   0x", f->lr);
    arm_fault_write_reg("\tPC:   0x", f->pc);
    arm_fault_write_reg("\tXPSR: 0x", f->xpsr);
    _exit(1);
}

void __attribute__((naked))
arm_hardfault_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_HARDFAULT));
    __asm__("bl  arm_fault");
}

void __attribute__((naked))
arm_memmange_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_MEMMANAGE));
    __asm__("bl  arm_fault");
}

void __attribute__((naked))
arm_busfault_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_BUSFAULT));
    __asm__("bl  arm_fault");
}

void __attribute__((naked))
arm_usagefault_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_USAGE));
    __asm__("bl  arm_fault");
}

#else

struct fault {
    unsigned int        r[7];
    unsigned int        pc;
};

static const char *const reasons[] = {
    "undef\n",
    "svc\n",
    "prefetch_abort\n",
    "data_abort\n"
};

#define REASON_UNDEF            0
#define REASON_SVC              1
#define REASON_PREFETCH_ABORT   2
#define REASON_DATA_ABORT       3

static void __attribute__((used))
arm_fault(struct fault *f, int reason)
{
    int r;
    fputs("ARM fault: ", stdout);
    fputs(reasons[reason], stdout);
    char prefix[] = "\tR#:   0x";
    for (r = 0; r <= 6; r++) {
        prefix[2] = '0' + r;    /* overwrite # with register number */
        arm_fault_write_reg(prefix, f->r[r]);
    }
    arm_fault_write_reg("\tPC:   0x", f->pc);
    _exit(1);
}

#define VECTOR_COMMON \
    __asm__("mov r7, %0" : : "r" (__stack)); \
    __asm__("mov sp, r7"); \
    __asm__("push {lr}"); \
    __asm__("push {r0-r6}"); \
    __asm__("mov r0, sp")

void __attribute__((naked)) __section(".init")
arm_undef_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov r1, #" REASON(REASON_UNDEF));
    __asm__("bl  arm_fault");
}

void __attribute__((naked)) __section(".init")
arm_prefetch_abort_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov r1, #" REASON(REASON_UNDEF));
    __asm__("bl  arm_fault");
}

void __attribute__((naked)) __section(".init")
arm_data_abort_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov r1, #" REASON(REASON_UNDEF));
    __asm__("bl  arm_fault");
}

#endif

#endif

