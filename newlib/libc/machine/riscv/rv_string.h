/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.

   Changes from Qualcomm Technologies, Inc. are provided under the following
   license:
   Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
   SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#ifndef _RV_STRING_H_
#define _RV_STRING_H_

/* This file defines macros to choose a specific custom implementation for
 * 'string.h' APIs. The specialization is segretated per API to allow for
 * different and/or complex scenarios for each API. */

#include <picolibc.h>

#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
#define _MACHINE_RISCV_MEMCPY_ASM_
#else
#define _MACHINE_RISCV_MEMCPY_C_
#endif

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
#define _MACHINE_RISCV_MEMMOVE_GENERIC_
#else
#define _MACHINE_RISCV_MEMMOVE_ASM_
#endif

#endif /* _RV_STRING_H_ */
