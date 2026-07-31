/* Copyright (c) 2019  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include "rv_string.h"
#include "asm.h"

#ifdef _MACHINE_RISCV_MEMMOVE_VECTOR_
.section .text.memmove
.global memmove
.type memmove, @function
.option push
.option arch, +zve32x
/*
 * RISC-V Vector memmove
 *
 * Strategy (mirrors vector memcpy, with both directions):
 *   - n == 0 or dst == src     : nothing to do
 *   - src >= dst, OR dst-src >= n (no overlap) : forward copy
 *   - otherwise                : backward copy
 *
 *   Forward path:
 *     n < 16 small        : single e8 tail loop
 *     align dst to 4B with e8, then if src co-aligned (mod 4) use
 *     fast vle32/vse32 bulk; else fall back to e8 bulk.
 *     Final 0..3 byte tail with e8.
 *
 *   Backward path (mirror):
 *     n < 16 small        : single e8 backward tail loop
 *     align dst_end DOWN to 4B with one e8 store, then if src_end
 *     co-aligned (mod 4) use fast vle32/vse32 backward bulk;
 *     else e8 backward bulk. Final 0..3 byte head with e8.
 *
 * Registers:
 *   a0 = dst (return value, preserved)
 *   a1 = src
 *   a2 = n  (consumed)
 *   t0 = running dst (or dst_end going backward)
 *   t1 = running src (or src_end going backward)
 *   t2,t3,t4,t5 = scratch
 */
memmove:
  LPAD
  beqz   a2, .Ldone_move          /* n == 0 */
  beq    a0, a1, .Ldone_move      /* dst == src */

  /* Overlap check: forward is safe if src >= dst, or gap >= n */
  bgeu   a1, a0, .Lforward_move
  sub    t2, a0, a1               /* t2 = dst - src */
  bgeu   t2, a2, .Lforward_move
  /* fallthrough: must copy backward */

/* ============================================================
 * Backward copy: dst overlaps src from a higher address.
 * Walk from end toward start.
 * ============================================================ */
  add    t0, a0, a2               /* t0 = dst_end (one past last) */
  add    t1, a1, a2               /* t1 = src_end */

  /* Small: skip alignment dance, jump to backward byte tail loop */
  li     t3, 16
  bltu   a2, t3, .Lback_tail

  /* --- Align dst_end DOWN to 4 bytes --- */
  andi   t2, t0, 3                /* misalign = dst_end & 3 */
  beqz   t2, .Lback_check_src
  /* Copy `t2` byte(s) backward with e8. */
  vsetvli zero, t2, e8, m1, ta, ma
  sub    t0, t0, t2
  sub    t1, t1, t2
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  sub    a2, a2, t2

.Lback_check_src:
  /* dst_end now 4-aligned. Need src_end also 4-aligned for vse32. */
  andi   t2, t1, 3
  bnez   t2, .Lback_tail          /* not co-aligned then e8 fallback */

  /* Fast e32 backward path */
  srli   t3, a2, 2                /* t3 = remaining word count */
  beqz   t3, .Lback_tail
  andi   a2, a2, 3                /* precompute byte tail (head) */
.Lback_bulk_loop:
  vsetvli t2, t3, e32, m8, ta, ma /* t2 = words this iter */
  slli   t5, t2, 2                /* bytes this iter = words*4 */
  sub    t0, t0, t5
  sub    t1, t1, t5
  vle32.v v0, (t1)
  vse32.v v0, (t0)
  sub    t3, t3, t2
  bnez   t3, .Lback_bulk_loop
  /* fallthrough: 0..3 head bytes still to copy */

.Lback_tail:
  beqz   a2, .Ldone_move
.Lback_tail_loop:
  vsetvli t2, a2, e8, m8, ta, ma
  sub    t0, t0, t2
  sub    t1, t1, t2
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  sub    a2, a2, t2
  bnez   a2, .Lback_tail_loop
  ret

/* ============================================================
 * Forward copy: identical strategy to memcpy.
 * ============================================================ */
.Lforward_move:
  mv     t0, a0                   /* running dst */
  mv     t1, a1                   /* running src */

  /* Small copies: single e8 tail loop handles any alignment/size */
  li     t3, 16
  bltu   a2, t3, .Lfwd_tail

  /* --- Align dst UP to 4 bytes --- */
  andi   t2, t0, 3
  beqz   t2, .Lfwd_check_src
  li     t4, 4
  sub    t3, t4, t2               /* pad = 4 - (dst & 3) in {1,2,3} */
  vsetvli zero, t3, e8, m1, ta, ma
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  add    t0, t0, t3
  add    t1, t1, t3
  sub    a2, a2, t3

.Lfwd_check_src:
  /* dst now 4-aligned. Need src also 4-aligned for vse32. */
  andi   t2, t1, 3
  bnez   t2, .Lfwd_tail           /* not co-aligned then e8 fallback */

  /* Fast e32 forward path */
  srli   t3, a2, 2                /* t3 = word count */
  beqz   t3, .Lfwd_tail
  andi   a2, a2, 3                /* precompute byte tail */
.Lfwd_bulk_loop:
  vsetvli t2, t3, e32, m8, ta, ma
  vle32.v v0, (t1)
  vse32.v v0, (t0)
  sub    t3, t3, t2
  slli   t5, t2, 2
  add    t0, t0, t5
  add    t1, t1, t5
  bnez   t3, .Lfwd_bulk_loop
  /* fallthrough: 0..3 byte tail */

.Lfwd_tail:
  beqz   a2, .Ldone_move
.Lfwd_tail_loop:
  vsetvli t2, a2, e8, m8, ta, ma
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  add    t0, t0, t2
  add    t1, t1, t2
  sub    a2, a2, t2
  bnez   a2, .Lfwd_tail_loop

.Ldone_move:
  ret
  .size memmove, .-memmove
  .option pop
#endif /* _MACHINE_RISCV_MEMMOVE_VECTOR_ */

#ifdef _MACHINE_RISCV_MEMMOVE_ASM_
.section .text.memmove
.global memmove
.type	memmove, @function
memmove:
  LPAD
  beqz a2, .Ldone		/* in case there are 0 bytes to be copied, return immediately */

  mv a4, a0			/* copy the destination address over to a4, since memmove should return that address in a0 at the end */
  li a3, 1
  bgtu  a1, a0, .Lcopy		/* in case of source address > destination address, copy from start to end of the specified memory area */

  li a3, -1			/* otherwhise, start copying from the end of the specified memory area in order to prevent data loss in case of overlapping memory areas.*/
  add   a4, a4, a2		/* add the number of bytes to be copied to both addresses. this gives us the address one byte past the end of the memory area we want to copy, */
  add   a1, a1, a2		/* therefore we need to subtract 1 from both addresses in the next step before starting the copying process. */

.Lincrement:
  add   a4, a4, a3 		/* in case of source address < destination address, increment both addresses by -1 before copying any data to obtain the correct start addresses */
  add   a1, a1, a3
.Lcopy:
  lbu a5, 0(a1)
  addi   a2, a2, -1		/* copy bytes as long as a2 (= the number of bytes to be copied) > 0. the increment is done here to relax the RAW dependency between load and store */
  sb a5, 0(a4)
  bnez a2, .Lincrement

.Ldone:
  ret

  .size	memmove, .-memmove
#endif /* _MACHINE_RISCV_MEMMOVE_ASM_ */
