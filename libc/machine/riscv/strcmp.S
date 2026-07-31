/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include <picolibc.h>

#include "asm.h"

.section .text.strcmp
.globl strcmp
.type  strcmp, @function
strcmp:
  LPAD
#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
.Lcompare:
  lbu   a2, 0(a0)
  lbu   a3, 0(a1)
  addi   a0, a0, 1
  addi   a1, a1, 1
  bne   a2, a3, .Lreturn_diff
  bnez  a2, .Lcompare

.Lreturn_diff:
  sub   a0, a2, a3
  ret

.size	strcmp, .-strcmp
#else
  li    t2, -1

#if !(__riscv_misaligned_slow || __riscv_misaligned_fast)
  or    a4, a0, a1
  and   a4, a4, SZREG-1
  bnez  a4, .Lmisaligned
#endif

#ifndef __riscv_zbb
#if SZREG == 4
  li a5, 0x7f7f7f7f
#else
  ld a5, mask
#endif
#endif

/* -------------------------------------------------------------------
 * Register allocation per target:
 *
 *   Non-RV32E (32 regs): use a6, t3, t4, t5 freely.
 *   RV32E (16 regs) + Zbb: remap a6→a5, t3→t0, t4→t1 (a5 is unused
 *                          when Zbb provides orc.b, so it's free as
 *                          the second a1-word slot).
 *   RV32E + non-Zbb: not enough scratch (mask occupies a5, plus 4
 *                    data + 4 intermediates + (-1) const = 10 regs).
 *                    Fall through to single-word loop.
 * ------------------------------------------------------------------- */
#if defined(__riscv_e) || defined(__riscv_32e)
#  ifdef __riscv_zbb
#    define R_W1A a5    /* second a1 word (was a6) */
#    define R_O0  t0    /* orc.b result 0 (was t3) */
#    define R_O1  t1    /* orc.b result 1 (was t4) */
#    define STRCMP_PAIR 1
#  else
     /* RV32E + non-Zbb: not enough registers, use single-word loop */
#    define STRCMP_PAIR 0
#  endif
#else
#  define R_W1A a6
#  define R_O0  t3
#  define R_O1  t4
#  define R_TMP t5     /* non-Zbb intermediate (was t5) */
#  define STRCMP_PAIR 1
#endif

#if STRCMP_PAIR
  .macro check_word_pair i n
    /* --- Fused_LDD #1: two loads from base a0 (back-to-back) --- */
    REG_L a2, \i*SZREG(a0)
    REG_L a4, \i*SZREG+SZREG(a0)

    /* --- Interleave: orc.b on a2 dual-issues with Fused_LDD #2 below.
     *     orc.b depends on a2 (ready from Fused_LDD #1), independent of
     *     the a1 loads → MEM pipe + ALU pipe fire in parallel.        --- */
#ifdef __riscv_zbb
    orc.b R_O0, a2
#endif

    /* --- Fused_LDD #2: two loads from base a1 (back-to-back) --- */
    REG_L a3, \i*SZREG(a1)
    REG_L R_W1A, \i*SZREG+SZREG(a1)

    /* --- Second orc.b dual-issues with the first branch below --- */
#ifdef __riscv_zbb
    orc.b R_O1, a4
#else
    /* Non-Zbb path: keep original grouping (more ALU ops, can't interleave
     * cleanly without breaking Fused_LDD #2 — most have already paid the
     * MEM cost by then anyway). */
    and   R_O0, a2, a5
    and   R_O1, a4, a5
    or    t1,   a2, a5
    or    R_TMP, a4, a5
    add   R_O0, R_O0, a5
    add   R_O1, R_O1, a5
    or    R_O0, R_O0, t1
    or    R_O1, R_O1, R_TMP
#endif

    /* --- Branches grouped at end --- */
    bne   R_O0, t2, .Lnull\i
    bne   a2, a3, .Lmismatch
    bne   R_O1, t2, .Lnull_hi_\i
    .if \i+2-\n
      bne a4, R_W1A, .Lmismatch_hi_\i   /* a4 vs R_W1A */
    .else
      add a0, a0, \n*SZREG
      add a1, a1, \n*SZREG
      beq a4, R_W1A, .Lloop
      j   .Lmismatch_hi_\i
    .endif

.Lnull_hi_\i:
    mv    a2, a4
    mv    a3, R_W1A
    j     .Lnull\i\()_p1
.Lmismatch_hi_\i:
    mv    a2, a4
    mv    a3, R_W1A
    j     .Lmismatch
  .endm

  .macro foundnull i n
    .ifne \i
.Lnull\i:
      add   a0, a0, \i*SZREG
      add   a1, a1, \i*SZREG
    .else
.Lnull0:
    .endif
    bne   a2, a3, .Lmisaligned
    li    a0, 0
    ret
    .ifeq \i & 1
.Lnull\i\()_p1:
      add   a0, a0, \i*SZREG+SZREG
      add   a1, a1, \i*SZREG+SZREG
      bne   a2, a3, .Lmisaligned
      li    a0, 0
      ret
    .endif
  .endm

.Lloop:
  check_word_pair 0 2

#else  /* !STRCMP_PAIR : RV32E + non-Zbb fallback (single word) */

  .macro check_one_word i n
    REG_L a2, \i*SZREG(a0)
    REG_L a3, \i*SZREG(a1)
    and   a4, a2, a5
    or    t1, a2, a5
    add   a4, a4, a5
    or    a4, a4, t1
    bne   a4, t2, .Lnull\i
    .if \i+1-\n
      bne a2, a3, .Lmismatch
    .else
      add a0, a0, \n*SZREG
      add a1, a1, \n*SZREG
      beq a2, a3, .Lloop
      j   .Lmismatch
    .endif
  .endm

  .macro foundnull i n
    .ifne \i
.Lnull\i:
      add   a0, a0, \i*SZREG
      add   a1, a1, \i*SZREG
    .else
.Lnull0:
    .endif
    bne   a2, a3, .Lmisaligned
    li    a0, 0
    ret
  .endm

.Lloop:
  check_one_word 0 1

#endif /* STRCMP_PAIR */

.Lmismatch:
  # words don't match, but a2 has no null byte.
  #if __riscv_zbb
    # this is an implementation from  Chapter 29 of
    # Volume I: RISC-V Unprivileged ISA Specification
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      rev8  a2, a2
      rev8  a3, a3
    #endif
    sltu a0, a2, a3
    neg  a0, a0
    ori  a0, a0, 1
    ret

  #else
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      #if __riscv_xlen == 64
        sll   a4, a2, 48
        sll   a5, a3, 48
        bne   a4, a5, .Lmismatch_upper
        sll   a4, a2, 32
        sll   a5, a3, 32
        bne   a4, a5, .Lmismatch_upper
      #endif
      sll   a4, a2, 16
      sll   a5, a3, 16
      bne   a4, a5, .Lmismatch_upper

      srl   a4, a2, 8*SZREG-16
      srl   a5, a3, 8*SZREG-16
      sub   a0, a4, a5
      and   a1, a0, 0xff
      bnez  a1, .Lfinal_upper_diff
      ret

      .Lmismatch_upper:
        srl   a4, a4, 8*SZREG-16
        srl   a5, a5, 8*SZREG-16
        sub   a0, a4, a5
        and   a1, a0, 0xff
        bnez  a1, .Lfinal_upper_diff
        ret

      .Lfinal_upper_diff:
        and   a4, a4, 0xff
        and   a5, a5, 0xff
        sub   a0, a4, a5
        ret
    #else
      #if __riscv_xlen == 64
        srl   a4, a2, 48
        srl   a5, a3, 48
        bne   a4, a5, .Lmismatch_lower
        srl   a4, a2, 32
        srl   a5, a3, 32
        bne   a4, a5, .Lmismatch_lower
      #endif
      srl   a4, a2, 16
      srl   a5, a3, 16
      bne   a4, a5, .Lmismatch_lower

      srl	  a4, a2, 8
      srl   a5, a3, 8
      bne   a4, a5, .Lbyte_diff
      and   a4, a2, 0xff
      and   a5, a3, 0xff

      .Lbyte_diff:
        sub	a0, a4, a5
        ret

      .Lmismatch_lower:
        srl	  a2, a4, 8
        srl   a3, a5, 8
        bne   a2, a3, .Lfinal_lower_diff
        and   a2, a4, 0xff
        and   a3, a5, 0xff

      .Lfinal_lower_diff:
        sub	a0, a2, a3
        ret
    #endif
  #endif
	
.Lmisaligned:
  # misaligned
  lbu   a2, 0(a0)
  lbu   a3, 0(a1)
  add   a0, a0, 1
  add   a1, a1, 1
  bne   a2, a3, .Lmisaligned_diff
  bnez  a2, .Lmisaligned

.Lmisaligned_diff:
  sub   a0, a2, a3
  ret

  # cases in which a null byte was detected
#if STRCMP_PAIR
  foundnull 0 2
  foundnull 1 2
#else
  foundnull 0 1
#endif
#if !defined(__riscv_zbb) && SZREG != 4
.align 3
mask:
.dword 0x7f7f7f7f7f7f7f7f
#endif
.size	strcmp, .-strcmp

#endif
