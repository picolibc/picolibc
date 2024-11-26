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
#include <sys/cdefs.h>
#include <sys/types.h>

/* The size of the thread control block.
 * TLS relocations are generated relative to
 * a location this far *before* the first thread
 * variable (!)
 * NB: The actual size before tp also includes padding
 * to align up to the alignment of .tdata/.tbss.
 */
#if __SIZE_WIDTH__ == 32
extern char __arm32_tls_tcb_offset;
#define TP_OFFSET ((size_t)&__arm32_tls_tcb_offset)
#else
extern char __arm64_tls_tcb_offset;
#define TP_OFFSET ((size_t)&__arm64_tls_tcb_offset)
#endif

#pragma GCC diagnostic ignored "-Warray-bounds"

static inline void
_set_tls(void *tls)
{
	__asm__ volatile("msr tpidr_el0, %x0" : : "r" (tls - TP_OFFSET));
}

#include "../../crt0.h"

/* Defined in crt0.S */
#define MMU_BLOCK_COUNT       8
extern uint64_t __identity_page_table[MMU_BLOCK_COUNT];
extern void _start(void);
extern const void *__vector_table[];

#define SCTLR_MMU       (1 << 0)
#define SCTLR_A         (1 << 1)
#define SCTLR_C         (1 << 2)
#define SCTLR_ICACHE    (1 << 12)
#define SCTLR_WXN       (1 << 19)
#define TCR_T0SZ_BIT    0
#define TCR_EPD0        (1 << 7)
#define TCR_IRGN0_BIT   8
#define TCR_IRGN0_NC    (0 << TCR_IRGN0_BIT)
#define TCR_IRGN0_WB_WA (1 << TCR_IRGN0_BIT)
#define TCR_IRGN0_WT    (2 << TCR_IRGN0_BIT)
#define TCR_IRGN0_WB    (3 << TCR_IRGN0_BIT)
#define TCR_ORGN0_BIT   10
#define TCR_ORGN0_NC    (0 << TCR_ORGN0_BIT)
#define TCR_ORGN0_WB_WA (1 << TCR_ORGN0_BIT)
#define TCR_ORGN0_WT    (2 << TCR_ORGN0_BIT)
#define TCR_ORGN0_WB    (3 << TCR_ORGN0_BIT)
#define TCR_SH0_BIT     12
#define TCR_SH0_NS      (0 << TCR_SH0_BIT)
#define TCR_SH0_OS      (2 << TCR_SH0_BIT)
#define TCR_SH0_IS      (3 << TCR_SH0_BIT)
#define TCR_TG0_BIT     14
#define TCR_TG0_4KB     (0 << TCR_TG0_BIT)
#define TCR_TG0_64KB    (1 << TCR_TG0_BIT)
#define TCR_TG0_16KB    (2 << TCR_TG0_BIT)
#define TCR_EPD1        (1 << 23)
#define TCR_IPS_BIT     32
#define TCR_IPS_4GB     (0LL << TCR_IPS_BIT)

/* QEMU boots into EL1, and FVPs boot into EL3, so we need to use the correct
 * system registers. */
#if defined(MACHINE_qemu)
#define BOOT_EL "EL1"
#elif defined(MACHINE_fvp)
#define BOOT_EL "EL3"
#else
#error "Unknown machine type"
#endif

void _cstart(void)
{
        uint64_t        sctlr;

        /* Invalidate the cache */
        __asm__("ic iallu");
        __asm__("isb\n");

        /*
         * Set up the TCR register to provide a 33bit VA space using
         * 4kB pages over 4GB of PA
         */
        __asm__("msr    tcr_"BOOT_EL", %x0" ::
                "r" ((0x1f << TCR_T0SZ_BIT) |
                     TCR_IRGN0_WB_WA |
                     TCR_ORGN0_WB_WA |
                     TCR_SH0_IS |
                     TCR_TG0_4KB |
                     TCR_EPD1 |
                     TCR_IPS_4GB));

        /* Load the page table base */
        __asm__("msr    ttbr0_"BOOT_EL", %x0" :: "r" (__identity_page_table));

        /*
         * Set the memory attributions in the MAIR register:
         *
         * Region 0 is Normal memory
         * Region 1 is Device memory
         */
        __asm__("msr    mair_"BOOT_EL", %x0" ::
                "r" ((0xffLL << 0) | (0x00LL << 8)));

        /*
         * Enable caches, and the MMU, disable alignment requirements
         * and write-implies-XN
         */
        __asm__("mrs    %x0, sctlr_"BOOT_EL"" : "=r" (sctlr));
        sctlr |= SCTLR_ICACHE | SCTLR_C | SCTLR_MMU;
        #ifdef __ARM_FEATURE_UNALIGNED
            sctlr &= ~SCTLR_A;
        #else
            sctlr |= SCTLR_A;
        #endif
        sctlr &= ~SCTLR_WXN;
        __asm__("msr    sctlr_"BOOT_EL", %x0" :: "r" (sctlr));
        __asm__("isb\n");

        /* Set the vector base address register */
        __asm__("msr    vbar_"BOOT_EL", %x0" :: "r" (__vector_table));
	__start();
}


#ifdef CRT0_SEMIHOST

/*
 * Trap faults, print message and exit when running under semihost
 */

#include <semihost.h>
#include <unistd.h>
#include <stdio.h>

#define _REASON(r) #r
#define REASON(r) _REASON(r)

static void aarch64_fault_write_reg(const char *prefix, uint64_t reg)
{
    fputs(prefix, stdout);

    for (unsigned i = 0; i < 16; i++) {
        unsigned digitval = 0xF & (reg >> (60 - 4*i));
        char digitchr = '0' + digitval + (digitval >= 10 ? 'a'-'0'-10 : 0);
        putchar(digitchr);
    }

    putchar('\n');
}

struct fault {
    uint64_t    x[31];
    uint64_t    pc;
    uint64_t    esr;
    uint64_t    far;
};

static const char *const reasons[] = {
    "sync\n",
    "irq\n",
    "fiq\n",
    "serror\n"
};

/* Called from assembly wrappers in crt0.S, which fills *f with the register
 * values at the point the fault happened. */
void aarch64_fault(struct fault *f, int reason)
{
    int r;
    fputs("AARCH64 fault: ", stdout);
    fputs(reasons[reason], stdout);
    char prefix[] = "\tX##:   0x";
    for (r = 0; r <= 30; r++) {
        prefix[2] = '0' + r / 10;    /* overwrite # with register number */
        prefix[3] = '0' + r % 10;    /* overwrite # with register number */
        aarch64_fault_write_reg(prefix, f->x[r]);
    }
    aarch64_fault_write_reg("\tPC:    0x", f->pc);
    aarch64_fault_write_reg("\tESR:   0x", f->esr);
    aarch64_fault_write_reg("\tFAR:   0x", f->far);
    _exit(1);
}

#endif /* CRT0_SEMIHOST */
