/*
 * sme-abi.S - minimal SME ABI routines implementation
 *
 * Copyright (c) 2026, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "asmdefs.h"

#define FEAT_SME_1      (1LL << 24)
#define FEAT_SME_2      (1LL << 25)

ENTRY (__arm_has_sme)
  mrs x0, id_aa64pfr1_el1
  tst x0, (FEAT_SME_1|FEAT_SME_2)
  cset x0, ne
  ret
END (__arm_has_sme)

// __arm_sme_state fills the result registers based on id_aa64pfr1_el1.
ENTRY (__arm_sme_state)
  .variant_pcs __arm_sme_state

  stp x29, x30, [sp, #-16]!
  mov x29, sp

  bl __arm_has_sme
  mov x1, xzr

  /* Check if we have SME */
  cbz x0, L(no_sme)

  orr x0, xzr, #0xC000000000000000
  /* Read SVCR */
  mrs x16, s3_3_c4_c2_2
  bfxil x0, x16, #0, #2
  /* Read TPIDR2_EL0 */
  mrs x1, s3_3_c13_c0_5
  b L(done)
L(no_sme):
  mov x0, xzr
L(done):
  ldp x29, x30, [sp], #16
  ret
END (__arm_sme_state)
