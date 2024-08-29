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

/*
 * We need 4 1GB mappings to cover the usual Normal memory space,
 * which runs from 0x00000000 to 0x7fffffff along with the usual
 * Device space which runs from 0x80000000 to 0xffffffff. However,
 * it looks like the smallest VA we can construct is 8GB, so we'll
 * pad the space with invalid PTEs
 */
#define MMU_NORMAL_COUNT        2
#define MMU_DEVICE_COUNT        2
#define MMU_INVALID_COUNT       4
extern uint64_t __identity_page_table[MMU_NORMAL_COUNT + MMU_DEVICE_COUNT + MMU_INVALID_COUNT];

#define MMU_DESCRIPTOR_VALID    (1 << 0)
#define MMU_DESCRIPTOR_BLOCK    (0 << 1)
#define MMU_DESCRIPTOR_TABLE    (1 << 1)

#define MMU_BLOCK_XN            (1LL << 54)
#define MMU_BLOCK_PXN           (1LL << 53)
#define MMU_BLOCK_CONTIG        (1LL << 52)
#define MMU_BLOCK_DBM           (1LL << 51)
#define MMU_BLOCK_GP            (1LL << 50)

#define MMU_BLOCK_NT            (1 << 16)
#define MMU_BLOCK_OA_BIT        12
#define MMU_BLOCK_NG            (1 << 11)
#define MMU_BLOCK_AF            (1 << 10)
#define MMU_BLOCK_SH_BIT        8
#define MMU_BLOCK_SH_NS         (0 << MMU_BLOCK_SH_BIT)
#define MMU_BLOCK_SH_OS         (2 << MMU_BLOCK_SH_BIT)
#define MMU_BLOCK_SH_IS         (3 << MMU_BLOCK_SH_BIT)
#define MMU_BLOCK_AP_BIT        6
#define MMU_BLOCK_NS            (1 << 5)
#define MMU_BLOCK_ATTR_BIT      2

#define MMU_NORMAL_FLAGS        (MMU_DESCRIPTOR_VALID |         \
                                 MMU_DESCRIPTOR_BLOCK |         \
                                 MMU_BLOCK_AF |                 \
                                 MMU_BLOCK_SH_IS |              \
                                 (0 << MMU_BLOCK_ATTR_BIT))

#define MMU_DEVICE_FLAGS        (MMU_DESCRIPTOR_VALID | \
                                 MMU_DESCRIPTOR_BLOCK | \
                                 MMU_BLOCK_AF | \
                                 (1 << MMU_BLOCK_ATTR_BIT))

#define MMU_INVALID_FLAGS       0

__asm__(
    ".section .rodata\n"
    ".global __identity_page_table\n"
    ".balign 65536\n"
    "__identity_page_table:\n"
    ".set _i, 0\n"
    ".rept " __XSTRING(MMU_NORMAL_COUNT) "\n"
    "  .8byte (_i << 30) |" __XSTRING(MMU_NORMAL_FLAGS) "\n"
    "  .set _i, _i + 1\n"
    ".endr\n"
    ".set _i, 0\n"
    ".rept " __XSTRING(MMU_DEVICE_COUNT) "\n"
    "  .8byte (1 << 31) | (_i << 30) |" __XSTRING(MMU_DEVICE_FLAGS) "\n"
    "  .set _i, _i + 1\n"
    ".endr\n"
    ".set _i, 0\n"
    ".rept " __XSTRING(MMU_INVALID_COUNT) "\n"
    "  .8byte " __XSTRING(MMU_INVALID_FLAGS) "\n"
    "  .set _i, _i + 1\n"
    ".endr\n"
    ".size __identity_page_table, " __XSTRING((MMU_NORMAL_COUNT + MMU_DEVICE_COUNT + MMU_INVALID_COUNT) * 8) "\n"
);

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

extern const void *__vector_table[];

static void __attribute((used))
_cstart(void)
{
        uint64_t        sctlr_el1;

        /* Invalidate the cache */
        __asm__("ic iallu");
        __asm__("isb\n");

        /*
         * Set up the TCR register to provide a 33bit VA space using
         * 4kB pages over 4GB of PA
         */
        __asm__("msr    tcr_el1, %x0" ::
                "r" ((0x1f << TCR_T0SZ_BIT) |
                     TCR_IRGN0_WB_WA |
                     TCR_ORGN0_WB_WA |
                     TCR_SH0_IS |
                     TCR_TG0_4KB |
                     TCR_EPD1 |
                     TCR_IPS_4GB));

        /* Load the page table base */
        __asm__("msr    ttbr0_el1, %x0" :: "r" (__identity_page_table));

        /*
         * Set the memory attributions in the MAIR register:
         *
         * Region 0 is Normal memory
         * Region 1 is Device memory
         */
        __asm__("msr    mair_el1, %x0" ::
                "r" ((0xffLL << 0) | (0x00LL << 8)));

        /*
         * Enable caches, and the MMU, disable alignment requirements
         * and write-implies-XN
         */
        __asm__("mrs    %x0, sctlr_el1" : "=r" (sctlr_el1));
        sctlr_el1 |= SCTLR_ICACHE | SCTLR_C | SCTLR_MMU;
        sctlr_el1 &= ~(SCTLR_A | SCTLR_WXN);
        __asm__("msr    sctlr_el1, %x0" :: "r" (sctlr_el1));
        __asm__("isb\n");

        /* Set the vector base address register */
        __asm__("msr    vbar_el1, %x0" :: "r" (__vector_table));
	__start();
}

void __section(".text.init.enter")
_start(void)
{
        /* Switch to EL1 */
	__asm__("msr     SPSel, #1");

	/* Initialize stack */
	__asm__("adrp x1, __stack");
	__asm__("add  x1, x1, :lo12:__stack");
	__asm__("mov sp, x1");
#if __ARM_FP
	/* Enable FPU */
	__asm__("mov x1, #(0x3 << 20)");
	__asm__("msr cpacr_el1,x1");
#endif
	/* Jump into C code */
	__asm__("bl _cstart");
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
};

static const char *const reasons[] = {
    "sync\n",
    "irq\n",
    "fiq\n",
    "serror\n"
};

#define REASON_SYNC     0
#define REASON_IRQ      1
#define REASON_FIQ      2
#define REASON_SERROR   3

static void __attribute__((used))
aarch64_fault(struct fault *f, int reason)
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
    _exit(1);
}

#define VECTOR_COMMON \
    __asm__("sub sp, sp, #256"); \
    __asm__("str x0, [sp, #0]"); \
    __asm__("str x1, [sp, #8]"); \
    __asm__("str x2, [sp, #16]"); \
    __asm__("str x3, [sp, #24]"); \
    __asm__("str x4, [sp, #32]"); \
    __asm__("str x5, [sp, #40]"); \
    __asm__("str x6, [sp, #48]"); \
    __asm__("str x7, [sp, #56]"); \
    __asm__("str x8, [sp, #64]"); \
    __asm__("str x9, [sp, #72]"); \
    __asm__("str x10, [sp, #80]"); \
    __asm__("str x11, [sp, #88]"); \
    __asm__("str x12, [sp, #96]"); \
    __asm__("str x13, [sp, #104]"); \
    __asm__("str x14, [sp, #112]"); \
    __asm__("str x15, [sp, #120]"); \
    __asm__("str x16, [sp, #128]"); \
    __asm__("str x17, [sp, #136]"); \
    __asm__("str x18, [sp, #144]"); \
    __asm__("str x19, [sp, #152]"); \
    __asm__("str x20, [sp, #160]"); \
    __asm__("str x21, [sp, #168]"); \
    __asm__("str x22, [sp, #176]"); \
    __asm__("str x23, [sp, #184]"); \
    __asm__("str x24, [sp, #192]"); \
    __asm__("str x25, [sp, #200]"); \
    __asm__("str x26, [sp, #208]"); \
    __asm__("str x27, [sp, #216]"); \
    __asm__("str x28, [sp, #224]"); \
    __asm__("str x29, [sp, #232]"); \
    __asm__("str x30, [sp, #240]"); \
    __asm__("mrs x0, ELR_EL1\n"); \
    __asm__("str x0, [sp, #248]"); \
    __asm__("mov x0, sp")

void __section(".init")
aarch64_sync_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov x1, #" REASON(REASON_SYNC));
    __asm__("b  aarch64_fault");
}

void __section(".init")
aarch64_irq_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov x1, #" REASON(REASON_IRQ));
    __asm__("b  aarch64_fault");
}

void __section(".init")
aarch64_fiq_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov x1, #" REASON(REASON_FIQ));
    __asm__("b  aarch64_fault");
}

#endif /* CRT0_SEMIHOST */
