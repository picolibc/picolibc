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

#ifdef _MACHINE_RISCV_MEMCPY_VECTOR_
/*
 * RISC-V Vector memcpy
 *
 * Strategy:
 *   - n < small  : single e8 vector copy (handles any alignment, any size)
 *   - common case: align dst to 4 bytes, then bulk copy with vle32/vse32
 *   - if src and dst can't be co-aligned (different mod-4 offsets):
 *       fall back to e8 bulk copy (no alignment requirement on vle8)
 *   - tail < 4 bytes: single e8 copy
 *
 * Registers:
 *   a0 = dst (return value, preserved in t0)
 *   a1 = src
 *   a2 = n   (consumed during copy)
 *   t0 = running dst
 *   t1 = running src
 *   t2,t3,t4,t5 = scratch
 */

.section .text.memcpy
.global memcpy
.type memcpy, @function
.option push
.option arch, +zve32x
memcpy:
  mv     t0, a0                   /* t0 = running dst */
  mv     t1, a1                   /* t1 = running src */
  beqz   a2, .Ldone               /* if n == 0, return */

  /* Small copies: just one e8 vector op handles it (any alignment) */
  li      t3, 16
  bltu    a2, t3, .Lbyte_tail

  /* --- Align dst to 4 bytes (vle32 requires 4-byte aligned address) --- */
  andi    t2, t0, 3
  beqz    t2, .Lcheck_src
  li      t4, 4
  sub     t2, t4, t2     /* pad = 4 - (dst & 3) */
  vsetvli t3, t2, e8, m1, ta, ma
  vle8.v  v0, (t1)
  vse8.v  v0, (t0)
  add     t0, t0, t3
  add     t1, t1, t3
  sub     a2, a2, t3
  beqz    a2, .Ldone

.Lcheck_src:
  /*
   * dst is now 4-aligned. If src has the same mod-4 offset (i.e. is also
   * 4-aligned now), use the fast e32 path. Otherwise fall back to e8.
   */
  andi    t2, t1, 3
  bnez    t2, .Lbyte_tail  /* src unalignable → e8 fallback */

  /* Fast e32 path */
  srli    t3, a2, 2         /* t3 = remaining word count */
  beqz    t3, .Lbyte_tail
  andi    a2, a2, 3        /* precompute tail */
.Lbulk_e32_loop:
  vsetvli t2, t3, e32, m8, ta, ma
  vle32.v v0, (t1)
  vse32.v v0, (t0)
  sub     t3, t3, t2
  slli    t5, t2, 2         /* bytes processed this iter */
  add     t0, t0, t5
  add     t1, t1, t5
  bnez    t3, .Lbulk_e32_loop
  /* fall through to byte tail (a2 < 4) */

/* ============================================================
 * Byte tail: handles 1..VLMAX bytes, loops if more remain
 * Also entry point for very small copies.
 * ============================================================ */
.Lbyte_tail:
  beqz    a2, .Ldone
.Lbyte_tail_loop:
  vsetvli t2, a2, e8, m8, ta, ma
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  add    t0, t0, t2
  add    t1, t1, t2
  sub    a2, a2, t2
  bnez    a2, .Lbyte_tail_loop

.Ldone:
  ret

  .size memcpy, .-memcpy
  .option pop
#endif /* _MACHINE_RISCV_MEMCPY_VECTOR_ */

#ifdef _MACHINE_RISCV_MEMCPY_ASM_
.section .text.memcpy
.global memcpy
.type	memcpy, @function
memcpy:
  mv a3, a0
  beqz a2, 2f

1:
  lbu a4, 0(a1)
  sb a4, 0(a3)
  addi   a2, a2, -1
  addi   a3, a3, 1
  addi   a1, a1, 1
  bnez a2, 1b

2:
  ret

  .size	memcpy, .-memcpy
#endif /* _MACHINE_RISCV_MEMCPY_ASM_ */
