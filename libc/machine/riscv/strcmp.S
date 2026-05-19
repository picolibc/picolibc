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

  /*
   * Process TWO words per expansion so:
   *   - REG_L a2,(i+0)*SZREG(a0) + REG_L a4,(i+1)*SZREG(a0) → Fused_LDD
   *   - REG_L a3,(i+0)*SZREG(a1) + REG_L a6,(i+1)*SZREG(a1) → Fused_LDD
   *   - orc.b t3,a2 + orc.b t4,a4                          → parallel ALU dispatch
   *   - branches grouped at the end                         → don't break fusion
   *
   * \i = first word index of the pair (must be even)
   * \n = total words unrolled in the loop body
   */
  .macro check_word_pair i n
    /* --- Fused_LDD: two loads from base a0, consecutive offsets --- */
    REG_L a2, \i*SZREG(a0)              /* was: (\i+0)*SZREG(a0) */
    REG_L a4, \i*SZREG+SZREG(a0)        /* was: (\i+1)*SZREG(a0) */
    /* --- Fused_LDD: two loads from base a1, consecutive offsets --- */
    REG_L a3, \i*SZREG(a1)              /* was: (\i+0)*SZREG(a1) */
    REG_L a6, \i*SZREG+SZREG(a1)        /* was: (\i+1)*SZREG(a1) */

    /* --- Independent ALU ops: dual-issue on ALU pipes --- */
#ifdef __riscv_zbb
    orc.b t3, a2
    orc.b t4, a4
#else
    and   t3, a2, a5
    and   t4, a4, a5
    or    t1, a2, a5
    or    t5, a4, a5
    add   t3, t3, a5
    add   t4, t4, a5
    or    t3, t3, t1
    or    t4, t4, t5
#endif

    /* --- Branches grouped at end --- */
    bne   t3, t2, .Lnull\i             /* null in word i   (a2/a3 already correct) */
      bne   a2, a3, .Lmismatch
    bne   t4, t2, .Lnull_hi_\i         /* null in word i+1 */
    .if \i+2-\n
      bne a4, a6, .Lmismatch_hi_\i
    .else
      add a0, a0, \n*SZREG
      add a1, a1, \n*SZREG
      beq a4, a6, .Lloop
    .endif

    /* Word i+1 handlers: move pair-2 regs into a2/a3 and reuse shared paths */
.Lnull_hi_\i:
    mv    a2, a4
    mv    a3, a6
    j     .Lnull\i\()_p1               /* foundnull entry for offset (i+1) */
.Lmismatch_hi_\i:
    mv    a2, a4
    mv    a3, a6
    j     .Lmismatch
  .endm

  /*
   * foundnull macro now also emits the .Lnull<i>_p1 entry for the
   * second word of each pair (adjusts a0/a1 by (i+1)*SZREG).
   */
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
      add   a0, a0, \i*SZREG+SZREG      /* was: (\i+1)*SZREG */
      add   a1, a1, \i*SZREG+SZREG
      bne   a2, a3, .Lmisaligned
      li    a0, 0
      ret
    .endif
  .endm

.Lloop:
  /* 1 pair = 2 words per iter. Keeps Fused_LDD + parallel orc.b,
     but limits over-read to 1 word past null (same as original). */
  check_word_pair 0 2

  # backwards branch to .Lloop contained above

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
  foundnull 0 2
  foundnull 1 2
#if !defined(__riscv_zbb) && SZREG != 4
.align 3
mask:
.dword 0x7f7f7f7f7f7f7f7f
#endif
.size	strcmp, .-strcmp

#endif
