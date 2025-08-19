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

void __disable_sanitizer
#ifdef STACK_TLS
_cstart(void *tls)
#else
_start(void)
#endif
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

#if defined(__ARM_FEATURE_PAC_DEFAULT) || defined(__ARM_FEATURE_BTI_DEFAULT)
        uint32_t        control;
        __asm__("mrs %0, CONTROL" : "=r" (control));
#ifdef __ARM_FEATURE_PAC_DEFAULT
        control |= (3 << 6);
#endif
#ifdef __ARM_FEATURE_BTI_DEFAULT
        control |= (3 << 4);
#endif
        __asm__("msr CONTROL, %0" : : "r" (control));
#endif
#ifdef STACK_TLS
        __start(tls);
#else
	__start();
#endif
}

#else /*  __ARM_ARCH_PROFILE == 'M' */

#include "crtstart.h"

/*
 * Regular ARM has an 8-entry exception vector and starts without SP
 * initialized, so start is a naked function which sets up the stack
 * and then branches here.
 */

extern void __vector_table(void);

void _cstart(void *sp);

__noreturn __used __section(".init") __disable_sanitizer
void
_cstart(void *sp)
{
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

/* Set up exception table base address (register VBAR_ELx).
   Architectures earlier than v7 have the base address fixed. */
#if __ARM_ARCH >= 7 && __ARM_ARCH_PROFILE != 'R'
    __asm__("mcr p15, #0, %0, c12, c0, 0" : : "r"(__vector_table) :);
#endif

#ifdef __PICOCRT_ENABLE_MMU

#if __ARM_ARCH >= 7 && __ARM_ARCH_PROFILE != 'R'

#define SCTLR_MMU (1 << 0)
#define SCTLR_DATA_L2 (1 << 2)
#define SCTLR_BRANCH_PRED (1 << 11)
#define SCTLR_ICACHE (1 << 12)
#define SCTLR_TRE       (1 << 28)
#define SCTLR_A (1 << 1)
#define SCTLR_U (1 << 22)

        uint32_t        mmfr0;
        __asm__("mrc p15, 0, %0, c0, c1, 4" : "=r" (mmfr0));

        /* Check to see if the processor supports VMSAv7 or better */
        if ((mmfr0 & 0xf) >= 3)
        {
                /* We have to set up an identity map and enable the MMU for caches.
                 * Additionally, all page table entries are set to Domain 0, so set up DACR
                 * so that Domain zero has permission checks enabled rather than "deny all".
                 */

                /* Set DACR Domain 0 permissions checked */
                __asm__("mcr p15, 0, %0, c3, c0, 0\n" :: "r" (1));

                /*
                 * Write TTBR
                 *
                 * No DSB since tables are statically initialized and dcache is off.
                 * We or __identity_page_table with 0x3 to set the cacheable flag bits.
                 */
                __asm__("mcr p15, 0, %0, c2, c0, 0\n"
                        :: "r" ((uintptr_t)__identity_page_table | 0x3));

                /* Note: we assume Data+L2 cache has been invalidated by reset. */
                __asm__("mcr p15, 0, %0, c7, c5, 0\n" :: "r" (0)); /* ICIALLU: invalidate instruction cache */
                __asm__("mcr p15, 0, %0, c8, c7, 0\n" :: "r" (0)); /* TLBIALL: invalidate TLB */
                __asm__("mcr p15, 0, %0, c7, c5, 6\n" :: "r" (0)); /* BPIALL: invalidate branch predictor */
                __asm__("isb\n");

                /* Enable caches, branch prediction and the MMU. Disable TRE */
                uint32_t sctlr;
                __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
                sctlr |= SCTLR_ICACHE | SCTLR_BRANCH_PRED | SCTLR_DATA_L2 | SCTLR_MMU;
                #ifndef __ARM_FEATURE_UNALIGNED
                    sctlr |= SCTLR_A;
                    sctlr &= ~SCTLR_U;
                #endif
                sctlr &= ~SCTLR_TRE;
                __asm__("mcr p15, 0, %0, c1, c0, 0\n" :: "r" (sctlr));
                __asm__("isb\n");
        }
#endif

#endif /* __PICOCRT_ENABLE_MMU */

#ifdef STACK_TLS
	__start(sp);
#else
        (void) sp;
	__start();
#endif
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

static void
arm_fault_write_reg(const char *prefix, unsigned reg)
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

static void __used
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

void __naked __disable_sanitizer
arm_hardfault_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_HARDFAULT));
    __asm__("bl  arm_fault");
}

void __naked __disable_sanitizer
arm_memmange_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_MEMMANAGE));
    __asm__("bl  arm_fault");
}

void __naked __disable_sanitizer
arm_busfault_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_BUSFAULT));
    __asm__("bl  arm_fault");
}

void __naked __disable_sanitizer
arm_usagefault_isr(void)
{
    __asm__("mov r0, sp");
    __asm__("movs r1, #" REASON(REASON_USAGE));
    __asm__("bl  arm_fault");
}

#else /* __ARM_ARCH_PROFILE == 'M' */

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

static void __used
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
    __asm__("push {lr}");                               \
    __asm__("push {r0-r6}");                            \
    __asm__("mov r0, sp")

void __naked __section(".init")  __disable_sanitizer
arm_undef_vector(void)
{
    VECTOR_COMMON;
    __asm__("movs r1, #" REASON(REASON_UNDEF));
    __asm__("bl  arm_fault");
}

void __naked __section(".init")  __disable_sanitizer
arm_prefetch_abort_vector(void)
{
    VECTOR_COMMON;
    __asm__("movs r1, #" REASON(REASON_PREFETCH_ABORT));
    __asm__("bl  arm_fault");
}

void __naked __section(".init")  __disable_sanitizer
arm_data_abort_vector(void)
{
    VECTOR_COMMON;
    __asm__("movs r1, #" REASON(REASON_DATA_ABORT));
    __asm__("bl  arm_fault");
}

#endif /* else __ARM_ARCH_PROFILE == 'M' */

#endif /* CRT0_SEMIHOST */
