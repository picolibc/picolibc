/*
 * The default linker stript for standalone executables running on
 * ADSP-BF533 processor.
 *
 * Copyright (C) 2008 Analog Devices, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

MEMORY
{
  MEM_L1_CODE : ORIGIN = 0xFFA00000, LENGTH = 0x10000
  MEM_L1_CODE_CACHE : ORIGIN = 0xFFA10000, LENGTH = 0x4000
  MEM_L1_SCRATCH : ORIGIN = 0xFFB00000, LENGTH = 0x1000
  MEM_L1_DATA_B : ORIGIN = 0xFF900000, LENGTH = 0x8000
  MEM_L1_DATA_A : ORIGIN = 0xFF800000, LENGTH = 0x8000

  MEM_L2 : ORIGIN = 0xFEB00000, LENGTH = 0x0
}
