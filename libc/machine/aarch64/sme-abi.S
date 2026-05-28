/*
 * sme-abi.S - minimal SME ABI routines implementation
 *
 * Copyright (c) 2026, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include "asmdefs.h"

// __arm_sme_state fills the result registers based on id_aa64pfr1_el1.
ENTRY (__arm_sme_state)
  .variant_pcs __arm_sme_state
  mov x0, xzr
  mov x1, xzr

  /* Check if we have SME (global initialized in crt0.c) */
  adrp x16, __arm64_has_sme
  ldrb w16, [x16, :lo12:__arm64_has_sme]
  cbz w16, L(no_sme)

  orr x0, x0, #0xC000000000000000
  /* Read SVCR */
  mrs x16, s3_3_c4_c2_2
  bfxil x0, x16, #0, #2
  /* Read TPIDR2_EL0 */
  mrs x1, s3_3_c13_c0_5
L(no_sme):
  ret
END (__arm_sme_state)
