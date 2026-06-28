/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 picolibc contributors
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES.
 */

/*
 * cfiss-mmu.S -- Strong override of _init_cfiss.
 *
 * Sv32 MMU-based Zicfiss shadow-stack initialization for RV32, followed by
 * a privilege drop from M-mode to U-mode.  This is the QEMU-testable
 * reference implementation that runs in the `semihost-cfi' crt0 variant.
 *
 * The function:
 *   1. Builds a flat Sv32 page table: a 1024-entry root with 4 MiB megapage
 *      identity mappings, except for the single megapage containing the
 *      shadow stack which is split into a 4 KiB leaf table; the leaf entry
 *      for the shadow-stack page uses the Zicfiss reserved encoding
 *      (V=1, R=0, W=1, U=1, A=1, D=1).
 *   2. Loads satp with Sv32 mode + root PPN.
 *   3. Pre-populates the shadow stack with a zero sentinel (must happen
 *      before SSE is enabled).
 *   4. Enables shadow-stack support in M, S, and U environments
 *      (mseccfg.SSE, menvcfg.SSE, senvcfg.SSE).
 *   5. Sets ssp to the top of the shadow-stack page.
 *   6. Programs mstatus.MPP = U and mepc = caller's ra, then mret's to
 *      U-mode, returning to the caller.
 *
 * Memory needed: 2 pages (root + leaf) + 1 page (shadow stack) = 12 KiB.
 *
 * Limitations:
 *   - RV32 / Sv32 only.  RV64 / Sv39 is left as a follow-up.
 *   - Caller's BSS must already be zeroed before entry (this is the case
 *     when called from POST_MEMORY_SETUP()).
 */

#if defined(__riscv_shadow_stack) && __riscv_xlen == 32

.option arch, +zicsr

/* PTE flag bits */
.equ PTE_V, (1 << 0)
.equ PTE_R, (1 << 1)
.equ PTE_W, (1 << 2)
.equ PTE_X, (1 << 3)
.equ PTE_U, (1 << 4)
.equ PTE_A, (1 << 6)
.equ PTE_D, (1 << 7)

/*
 * Zicfiss shadow-stack PTE encoding:
 *   V=1, R=0, W=1, X=0, U=1, A=1, D=1
 * The R=0,W=1 combination is reserved by the base spec and is repurposed
 * by Zicfiss to mark a page as a shadow-stack page.  Only sspush, sspopchk,
 * and ssamoswap may write such a page; ordinary stores fault.
 */
.equ PTE_SHADOW_STACK, (PTE_V | PTE_W | PTE_U | PTE_A | PTE_D)
.equ PTE_USER_RWX,     (PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D)

.equ PAGE_SIZE,     4096

/* mstatus bits */
.equ MSTATUS_MPP_MASK, (3 << 11)
.equ MSTATUS_MPIE,     (1 << 7)

/* xenvcfg / mseccfg bits */
.equ MENVCFG_SSE, (1 << 3)
.equ SENVCFG_SSE, (1 << 3)
.equ MSECCFG_SSE, (1 << 3)

/* ------------------------------------------------------------------------- */
/* Static storage (BSS, page-aligned)                                        */
/* ------------------------------------------------------------------------- */
.section .bss
.balign PAGE_SIZE
.local _cfiss_shadow_stack
_cfiss_shadow_stack:
    .space PAGE_SIZE                /* 1024 slots × 4 bytes on RV32 */

.balign PAGE_SIZE
.local _cfiss_pt_root
_cfiss_pt_root:
    .space (1024 * 4)               /* Level-1 root table */

.balign PAGE_SIZE
.local _cfiss_pt_leaf
_cfiss_pt_leaf:
    .space (1024 * 4)               /* Level-0 leaf table */

/* ------------------------------------------------------------------------- */
/* _init_cfiss                                                               */
/*                                                                           */
/* Strong override of the weak no-op default in crt0.c.  Called from         */
/* POST_MEMORY_SETUP() in M-mode after .bss has been zeroed.  Does not       */
/* "return" through ra; instead uses mret to resume at ra in U-mode.         */
/* ------------------------------------------------------------------------- */
.section .text
.global _init_cfiss
.type _init_cfiss, @function
_init_cfiss:
    /* Save ra (return target for mret) and sp (preserved across mret). */
    mv      t6, ra
    mv      t5, sp

    /* ---- 1. Build Sv32 page tables (identity map) -------------------- */

    /* Compute VPN indices for the shadow stack page:
     *   ssVpn1 = shadow_stack >> 22
     *   ssVpn0 = (shadow_stack >> 12) & 0x3FF
     */
    la      t0, _cfiss_shadow_stack
    srli    t1, t0, 22              /* t1 = ssVpn1 */
    srli    t2, t0, 12
    andi    t2, t2, 0x3FF           /* t2 = ssVpn0 */

    /* Fill the root table.  For each index i in [0,1024):
     *   - i == ssVpn1: non-leaf pointing at the leaf table
     *   - else:        4 MiB leaf megapage PTE = (i << 20) | PTE_USER_RWX
     */
    la      a0, _cfiss_pt_root      /* a0: cursor */
    li      a1, 0                   /* a1: loop index i */
    li      a2, 1024                /* a2: bound */
    la      a3, _cfiss_pt_leaf
    srli    a3, a3, 2
    ori     a3, a3, PTE_V           /* a3: non-leaf PTE -> leaf table */
    li      a4, PTE_USER_RWX
.L_root_loop:
    beq     a1, a2, .L_root_done
    beq     a1, t1, .L_root_ss
    slli    a5, a1, 20
    or      a5, a5, a4
    sw      a5, 0(a0)
    j       .L_root_next
.L_root_ss:
    sw      a3, 0(a0)
.L_root_next:
    addi    a0, a0, 4
    addi    a1, a1, 1
    j       .L_root_loop
.L_root_done:

    /* Fill the leaf table that covers the shadow-stack megapage.
     * megaBase = ssVpn1 << 22
     * For each i in [0,1024):
     *   pa     = megaBase + (i << 12)
     *   flags  = PTE_SHADOW_STACK if i == ssVpn0 else PTE_USER_RWX
     *   PTE    = (pa >> 2) | flags
     */
    la      a0, _cfiss_pt_leaf
    li      a1, 0
    slli    a3, t1, 22              /* a3 = megaBase */
    li      a4, PTE_USER_RWX
    li      a5, PTE_SHADOW_STACK
.L_l0_loop:
    beq     a1, a2, .L_l0_done
    slli    a6, a1, 12
    add     a6, a3, a6              /* pa */
    srli    a6, a6, 2               /* PPN */
    beq     a1, t2, .L_l0_ss
    or      a6, a6, a4
    j       .L_l0_store
.L_l0_ss:
    or      a6, a6, a5
.L_l0_store:
    sw      a6, 0(a0)
    addi    a0, a0, 4
    addi    a1, a1, 1
    j       .L_l0_loop
.L_l0_done:

    /* ---- 2. Activate Sv32 -------------------------------------------- */
    /* satp = (1 << 31) | (PtRoot >> 12)                                   */
    la      t0, _cfiss_pt_root
    srli    t0, t0, 12
    lui     t3, 0x80000             /* 1u << 31 */
    or      t0, t0, t3
    csrw    satp, t0
    sfence.vma

    /* ---- 3. Pre-populate the shadow stack with a zero sentinel ------- */
    la      t0, _cfiss_shadow_stack
    li      t3, PAGE_SIZE
    add     t0, t0, t3              /* top of page */
    addi    t0, t0, -4              /* one slot down */
    sw      zero, 0(t0)             /* sentinel: 0 */

    /* ---- 4. Enable shadow-stack support in M/S/U envcfg --------------- */
    /* mseccfg.SSE must be set before M-mode accesses ssp.                 */
    csrr    t3, 0x747               /* mseccfg */
    ori     t3, t3, MSECCFG_SSE
    csrw    0x747, t3

    csrr    t3, menvcfg
    ori     t3, t3, MENVCFG_SSE
    csrw    menvcfg, t3

    csrr    t3, senvcfg
    ori     t3, t3, SENVCFG_SSE
    csrw    senvcfg, t3

    /* ---- 5. Set ssp -------------------------------------------------- */
    csrw    ssp, t0

    /* ---- 6. mret to U-mode at original ra ---------------------------- */
    csrr    t0, mstatus
    li      t3, (~MSTATUS_MPP_MASK) & 0xFFFFFFFF
    and     t0, t0, t3              /* MPP = 00 (U-mode) */
    ori     t0, t0, MSTATUS_MPIE
    csrw    mstatus, t0
    csrw    mepc, t6
    mv      sp, t5
    mret

.size _init_cfiss, .-_init_cfiss

#endif /* __riscv_shadow_stack && __riscv_xlen == 32 */
