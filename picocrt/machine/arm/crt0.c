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
    "hardfault",
    "memmanage",
    "busfault",
    "usagefault"
};

#define REASON_HARDFAULT        0
#define REASON_MEMMANAGE        1
#define REASON_BUSFAULT         2
#define REASON_USAGE            3

static void __attribute__((used))
arm_fault(struct fault *f, int reason)
{
    printf("ARM fault: %s\n", reasons[reason]);
    printf("\tR0:   0x%08x\n", f->r0);
    printf("\tR1:   0x%08x\n", f->r1);
    printf("\tR2:   0x%08x\n", f->r2);
    printf("\tR3:   0x%08x\n", f->r3);
    printf("\tR12:  0x%08x\n", f->r12);
    printf("\tLR:   0x%08x\n", f->lr);
    printf("\tPC:   0x%08x\n", f->pc);
    printf("\tXPSR: 0x%08x\n", f->xpsr);
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
    "undef",
    "svc",
    "prefetch_abort",
    "data_abort"
};

#define REASON_UNDEF            0
#define REASON_SVC              1
#define REASON_PREFETCH_ABORT   2
#define REASON_DATA_ABORT       3

static void __attribute__((used))
arm_fault(struct fault *f, int reason)
{
    int r;
    printf("ARM fault: %s\n", reasons[reason]);
    for (r = 0; r <= 6; r++)
        printf("\tR%d:   0x%08x\n", r, f->r[r]);
    printf("\tPC:   0x%08x\n", f->pc);
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

