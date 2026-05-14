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


#define BYTE_TBL_SZ   31
#define WORD_TBL_SZ   32

#if __riscv_zilsd
/* Move size */
#define MV_SZ         8

/* Store instruction */
#define RG_ST         sd

/* Zilsd and Zclsd require an even numbered register */
#define REG_SPLAT     a4
#else
#define MV_SZ         SZREG
#define RG_ST         REG_S
#define REG_SPLAT     a1
#endif

/*
  Use an extended register for Zilsd and Zclsd if available
  since a5 is used for the odd numbered register, in order
  to eliminate an li instruction
*/
#if __riscv_zilsd && !__riscv_abi_rve
#define REG_TABLE     a6
#else
#define REG_TABLE     a5
#endif


.section .text.memset
.global memset
.type	memset, @function
memset:
#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  mv     a3, a0
  beqz   a2, .Ldone

.Lset:
  sb     a1, 0(a3)
  addi   a2, a2, -1
  addi   a3, a3, 1
  bnez   a2, .Lset

.Ldone:
  ret

#elif defined(__riscv_vector)
/*
 * Vector memset.
 *
 *   - Broadcast the fill byte ONCE into v0..v7 using e8/m8.
 *     Every byte of the group holds the fill value, so subsequent
 *     vse32 stores naturally write the correct splatted word
 *     (byte | byte<<8 | byte<<16 | byte<<24) without any extra
 *     scalar splat arithmetic.
 *
 *   - n < 16        : skip the alignment dance, go straight to the
 *                     e8 tail loop (handles any alignment / size).
 *   - common case   : align dst to 4 bytes with one e8 store, then
 *                     bulk fill with vse32 (4x larger vl per iter
 *                     than vse8 at the same LMUL).
 *   - byte tail     : single e8 loop covers 0..3 bytes after bulk
 *                     and also the < 16 small-size bypass.
 *
 * Registers:
 *   a0 = dst (return value, preserved)
 *   a1 = fill byte
 *   a2 = n   (consumed)
 *   t0 = running dst
 *   t2,t3,t4,t5 = scratch
 *   v0..v7 = broadcast fill
 */
.option push
.option arch, +zve32x
  mv     t0, a0                    /* t0 = running dst */
  beqz   a2, .Ldone_set

  /* Broadcast fill byte ONCE across v0..v7. */
  vsetvli t3, zero, e8, m8, ta, ma
  vmv.v.x v0, a1

  /* Small: skip alignment, go directly to byte tail loop. */
  li      t3, 16
  bltu    a2, t3, .Lset_tail

  /* --- Align dst to 4 bytes (required for fast vse32) --- */
  andi    t2, t0, 3
  beqz    t2, .Lset_bulk
  li      t4, 4
  sub     t3, t4, t2                /* pad = 4 - (dst & 3), in {1,2,3} */
  vsetvli zero, t3, e8, m1, ta, ma
  vse8.v  v0, (t0)
  add     t0, t0, t3
  sub     a2, a2, t3

.Lset_bulk:
  srli    t3, a2, 2                 /* t3 = remaining word count */
  andi    a2, a2, 3                 /* a2 = byte tail (0..3) */
  beqz    t3, .Lset_tail
.Lset_bulk_loop:
  vsetvli t2, t3, e32, m8, ta, ma   /* t2 = words this iter */
  vse32.v v0, (t0)
  sub     t3, t3, t2
  slli    t5, t2, 2                 /* bytes = words * 4 */
  add     t0, t0, t5
  bnez    t3, .Lset_bulk_loop

  /* --- Byte tail: also entry for n < 16 small bypass --- */
.Lset_tail:
  beqz    a2, .Ldone_set
.Lset_tail_loop:
  vsetvli t2, a2, e8, m8, ta, ma
  vse8.v  v0, (t0)
  add     t0, t0, t2
  sub     a2, a2, t2
  bnez    a2, .Lset_tail_loop

.Ldone_set:
  ret
.option pop

#else
  li     REG_TABLE, BYTE_TBL_SZ
  mv     a3, a0

  /* If there aren't many bytes, copy them individually to reduce overhead */
  bleu   a2, REG_TABLE, .Lcopy_bytes

  and    a4, a3, MV_SZ - 1
  beqz   a4, .Lword_check

  /*
    Jump into the byte table depending on the number of bytes that need to be
    written
  */
1:
  auipc  t0, %pcrel_hi(.Ltable_misaligned)

  /*
    Instructions in the tables are forced to be four bytes, so scale count
    by 4
  */
#if __riscv_zba
  sh2add t0, a4, t0
#else
  sll    t1, a4, 2
  add    t0, t0, t1
#endif

  /* Save the return address because we aren't exiting the function yet */
  mv     t1, ra
  jalr   t0, %pcrel_lo(1b)

  /* Update pointer and count by what was written */
  mv     ra, t1
  add    a4, a4, -MV_SZ
  add    a2, a2, a4
  sub    a3, a3, a4

  /* Access is now aligned. Check we can copy words. */
  bleu   a2, REG_TABLE, .Lcopy_bytes

.Lword_check:
  /* Don't need to splat special case of zero */
  bnez   a1, .Lsplat_byte
#if __riscv_zilsd
  mv     REG_SPLAT, a1
#endif
  j      .Lcopy_words_init

/*
  Align labels to four bytes after unconditional jumps to avoid any
  penalties when jumping to 32-bit instructions that aren't 4-byte
  aligned
*/
.p2align 2
.Lsplat_byte:
#if __riscv_zbkb
  packh  REG_SPLAT, a1, a1
#if __riscv_xlen == 64
  packw  REG_SPLAT, REG_SPLAT, REG_SPLAT
#endif
  pack   REG_SPLAT, REG_SPLAT, REG_SPLAT
#else
  and    a1, a1, 0xFF
  sll    t0, a1, 8
  or     a1, a1, t0
  sll    t0, a1, 16
  or     REG_SPLAT, a1, t0
#if __riscv_xlen == 64
  sll    t0, REG_SPLAT, 32
  or     REG_SPLAT, REG_SPLAT, t0
#endif
#endif

.Lcopy_words_init:
#if __riscv_zilsd
  /* Odd register of even-odd pair */
  mv     a5, REG_SPLAT
#endif

  /* Calculate end address */
  and    t0, a2, ~(MV_SZ - 1)
  add    t1, a3, t0

  /*
    The idea behind the table of word copies is that first we calculate any
    remainder of bytes that need to be copied by the table that aren't an
    entire table length. That's copied first. After that, runs of the entire
    table are performed.
  */
  and    t0, t0, (WORD_TBL_SZ - 1) * MV_SZ

  /* Skip if there's no remainder */
  beqz   t0, .Ltable_bigly
  neg    t0, t0
  add    t0, t0, WORD_TBL_SZ * MV_SZ

  /* Adjust start address with offset */
  sub    a3, a3, t0

1:
  auipc  t2, %pcrel_hi(.Ltable_bigly)

#if MV_SZ == 8
  /*
    If eight bytes are being copied with each store, we need to divide
    the table offset in half
  */
  srl    t0, t0, 1
#endif

  add    t2, t2, t0
  jr     t2, %pcrel_lo(1b)

.p2align 2
.Ltable_bigly:
/*
  Force the instructions to be four bytes to avoid an extra instruction
  that would be needed to halve the offset for sw
*/
.option push
.option norvc
  RG_ST  REG_SPLAT, MV_SZ*0(a3)
  RG_ST  REG_SPLAT, MV_SZ*1(a3)
  RG_ST  REG_SPLAT, MV_SZ*2(a3)
  RG_ST  REG_SPLAT, MV_SZ*3(a3)
  RG_ST  REG_SPLAT, MV_SZ*4(a3)
  RG_ST  REG_SPLAT, MV_SZ*5(a3)
  RG_ST  REG_SPLAT, MV_SZ*6(a3)
  RG_ST  REG_SPLAT, MV_SZ*7(a3)
  RG_ST  REG_SPLAT, MV_SZ*8(a3)
  RG_ST  REG_SPLAT, MV_SZ*9(a3)
  RG_ST  REG_SPLAT, MV_SZ*10(a3)
  RG_ST  REG_SPLAT, MV_SZ*11(a3)
  RG_ST  REG_SPLAT, MV_SZ*12(a3)
  RG_ST  REG_SPLAT, MV_SZ*13(a3)
  RG_ST  REG_SPLAT, MV_SZ*14(a3)
  RG_ST  REG_SPLAT, MV_SZ*15(a3)
  RG_ST  REG_SPLAT, MV_SZ*16(a3)
  RG_ST  REG_SPLAT, MV_SZ*17(a3)
  RG_ST  REG_SPLAT, MV_SZ*18(a3)
  RG_ST  REG_SPLAT, MV_SZ*19(a3)
  RG_ST  REG_SPLAT, MV_SZ*20(a3)
  RG_ST  REG_SPLAT, MV_SZ*21(a3)
  RG_ST  REG_SPLAT, MV_SZ*22(a3)
  RG_ST  REG_SPLAT, MV_SZ*23(a3)
  RG_ST  REG_SPLAT, MV_SZ*24(a3)
  RG_ST  REG_SPLAT, MV_SZ*25(a3)
  RG_ST  REG_SPLAT, MV_SZ*26(a3)
  RG_ST  REG_SPLAT, MV_SZ*27(a3)
  RG_ST  REG_SPLAT, MV_SZ*28(a3)
  RG_ST  REG_SPLAT, MV_SZ*29(a3)
  RG_ST  REG_SPLAT, MV_SZ*30(a3)
  RG_ST  REG_SPLAT, MV_SZ*31(a3)
.option pop

  /* Update the pointer and copy data if needed */
  add    a3, a3, MV_SZ * WORD_TBL_SZ
  bltu   a3, t1, .Ltable_bigly

  /* Copy any remaining bytes */
  and    a2, a2, MV_SZ - 1
  beqz   a2, .Lexit

#if __riscv_zilsd && __riscv_abi_rve
  /* Restore table size if necessary */
  li     REG_TABLE, BYTE_TBL_SZ
#endif

.Lcopy_bytes:
  auipc  t0, %pcrel_hi(.Ltable_tiny)

  sub    a2, REG_TABLE, a2

  /*
    Instructions in the tables are forced to be four bytes, so scale count
    by 4
  */
#if __riscv_zba
  sh2add t0, a2, t0
#else
  sll    a2, a2, 2
  add    t0, t0, a2
#endif

  /* Don't save the return address because we're exiting after the jump */
  jr     t0, %pcrel_lo(.Lcopy_bytes)

.p2align 2
.Ltable_tiny:
/*
  norvc is needed because the immediate is only two bits in size for c.sb,
  and without it the table would have a mix of 2- and 4-byte instructions
  when Zcb is available
*/
.option push
.option norvc
  sb     a1, 30(a3)
  sb     a1, 29(a3)
  sb     a1, 28(a3)
  sb     a1, 27(a3)
  sb     a1, 26(a3)
  sb     a1, 25(a3)
  sb     a1, 24(a3)
  sb     a1, 23(a3)
  sb     a1, 22(a3)
  sb     a1, 21(a3)
  sb     a1, 20(a3)
  sb     a1, 19(a3)
  sb     a1, 18(a3)
  sb     a1, 17(a3)
  sb     a1, 16(a3)
  sb     a1, 15(a3)
  sb     a1, 14(a3)
  sb     a1, 13(a3)
  sb     a1, 12(a3)
  sb     a1, 11(a3)
  sb     a1, 10(a3)
  sb     a1, 9(a3)
  sb     a1, 8(a3)
#if MV_SZ == 8
.Ltable_misaligned:
#endif
  sb     a1, 7(a3)
  sb     a1, 6(a3)
  sb     a1, 5(a3)
  sb     a1, 4(a3)
#if MV_SZ == 4
.Ltable_misaligned:
#endif
  sb     a1, 3(a3)
  sb     a1, 2(a3)
  sb     a1, 1(a3)
  sb     a1, 0(a3)
.option pop
.Lexit:
  ret
#endif
.size  memset, .-memset
