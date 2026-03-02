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

#ifdef _MACHINE_RISCV_MEMMOVE_VECTOR_
.section .text.memmove
.global memmove
.type memmove, @function
.option push
.option arch, +zve32x
memmove:
  beqz   a2, .Ldone_move          /* n == 0 */
  beq    a0, a1, .Ldone_move      /* dst == src */

  /* overlap check */
  bgeu   a1, a0, .Lforward_move   /* src >= dst then forward move */
  sub    t2, a0, a1               /* t2 = dst - src */
  bgeu   t2, a2, .Lforward_move   /* no overlap then forward move */

  /* backward move */
  add    t0, a0, a2               /* running dst_end */
  add    t1, a1, a2               /* running src_end */
  /* Align dst_end to SZREG: skip when __riscv_misaligned_fast, else align */
#ifndef __riscv_misaligned_fast
  /* process small data directly with vectors, no alignment optimization */
  li     t3, 32
  bltu   a2, t3, .Lbackward_loop

#if __riscv_xlen == 64
  andi   t2, t0, 7                /* misalignment = dst_end & 7 */
#else
  andi   t2, t0, 3                /* misalignment = dst_end & 3 */
#endif
  beqz   t2, .Lbackward_aligned   /* already aligned */
  /* copy tail bytes to reach aligned dst_end */
  vsetvli t3, t2, e8, m8, ta, ma
  sub    t0, t0, t3
  sub    t1, t1, t3
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  sub    a2, a2, t3
.Lbackward_aligned:
#endif
.Lbackward_loop:
  vsetvli t3, a2, e8, m8, ta, ma   /* t3 = vl (bytes) */
  sub    t0, t0, t3
  sub    t1, t1, t3
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  sub    a2, a2, t3
  bnez   a2, .Lbackward_loop
  ret

  /* forward move, same as memcpy */
.Lforward_move:
  mv     t0, a0                   /* running dst */
  mv     t1, a1                   /* running src */
  /* Align dst to SZREG: skip when __riscv_misaligned_fast, else align */
#ifndef __riscv_misaligned_fast
  /* process small data directly with vectors, no alignment optimization */
  li     t3, 32
  bltu   a2, t3, .Lforward_loop

#if __riscv_xlen == 64
  andi   t2, t0, 7                /* t2 = dst & 7 */
  beqz   t2, .Lforward_aligned    /* already aligned to 8 bytes */
  li     t4, 8
  sub    t2, t4, t2               /* pad = 8 - (dst & 7) */
#else
  andi   t2, t0, 3                /* t2 = dst & 3 */
  beqz   t2, .Lforward_aligned    /* already aligned to 4 bytes */
  li     t4, 4
  sub    t2, t4, t2               /* pad = 4 - (dst & 3) */
#endif
  /* copy prologue using vectors */
  vsetvli t3, t2, e8, m8, ta, ma
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  add    t0, t0, t3
  add    t1, t1, t3
  sub    a2, a2, t3
.Lforward_aligned:
#endif
.Lforward_loop:
  vsetvli t3, a2, e8, m8, ta, ma
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  add    t0, t0, t3
  add    t1, t1, t3
  sub    a2, a2, t3
  bnez   a2, .Lforward_loop
  /* fallthrough */

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
