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
.section .text.memcpy
.global memcpy
.type memcpy, @function
.option push
.option arch, +zve32x
memcpy:
  mv     t0, a0                   /* t0 = running dst */
  mv     t1, a1                   /* t1 = running src */
  beqz   a2, .Ldone               /* if n == 0, return */

  /* Align dst to SZREG: skip when __riscv_misaligned_fast, else align */
#ifndef __riscv_misaligned_fast
  /* process small data directly with vectors, no alignment optimization */
  li     t3, 32
  bltu   a2, t3, .Lbulk_copy
#if __riscv_xlen == 64
  andi   t2, t0, 7                /* t2 = dst & 7 */
  beqz   t2, .Lbulk_copy       /* already aligned to 8 bytes */
  li     t4, 8
  sub    t2, t4, t2               /* pad = 8 - (dst & 7) */
#else
  andi   t2, t0, 3                /* t2 = dst & 3 */
  beqz   t2, .Lbulk_copy       /* already aligned to 4 bytes */
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
  beqz   a2, .Ldone
#endif

.Lbulk_copy:
  vsetvli t2, a2, e8, m8, ta, ma
  vle8.v v0, (t1)
  vse8.v v0, (t0)
  add    t0, t0, t2
  add    t1, t1, t2
  sub    a2, a2, t2
  bnez   a2, .Lbulk_copy
  /* fallthrough */

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
