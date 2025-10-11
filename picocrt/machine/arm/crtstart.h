/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "picolibc.h"

#if __ARM_ARCH_PROFILE == 'M'

#else

#define MODE_USR        (0x10)
#define MODE_FIQ        (0x11)
#define MODE_IRQ        (0x12)
#define MODE_SVC        (0x13)
#define MODE_MON        (0x16)
#define MODE_ABT        (0x17)
#define MODE_HYP        (0x1a)
#define MODE_UND        (0x1b)
#define MODE_SYS        (0x1f)
#define I_BIT           (1 << 7)
#define F_BIT           (1 << 6)
#define T_BIT           (1 << 5)

#define MAKE_MODE(mode) (mode | I_BIT | F_BIT)

#define SHADOW_STACK_SIZE 0x10
#define STACK_IRQ (__stack - SHADOW_STACK_SIZE * 0)
#define STACK_ABT (__stack - SHADOW_STACK_SIZE * 1)
#define STACK_UND (__stack - SHADOW_STACK_SIZE * 2)
#define STACK_FIQ (__stack - SHADOW_STACK_SIZE * 3)
#define STACK_SYS (__stack - SHADOW_STACK_SIZE * 4)
#define STACK_SVC (__stack - SHADOW_STACK_SIZE * 5)

#ifdef __PICOCRT_ENABLE_MMU

#if __ARM_ARCH >= 7 && __ARM_ARCH_PROFILE != 'R'

/*
 * We need 4096 1MB mappings to cover the usual Normal memory space,
 * which runs from 0x00000000 to 0x7fffffff along with the usual
 * Device space which runs from 0x80000000 to 0xffffffff.
 */
#define MMU_NORMAL_COUNT 2048
#define MMU_DEVICE_COUNT 2048
#ifndef _ASM
extern uint32_t __identity_page_table[MMU_NORMAL_COUNT + MMU_DEVICE_COUNT];
#endif


/* Bits within a short-form section PTE (1MB mapping) */
#define MMU_NS_BIT      19
#define MMU_NG_BIT      17
#define MMU_S_BIT       16
#define MMU_AP2_BIT     15
#define MMU_TEX_BIT     12
#define MMU_AP0_BIT     10
#define MMU_XN_BIT      4
#define MMU_BC_BIT      2
#define MMU_PXN_BIT     0

#define MMU_TYPE_1MB    (0x2 << 0)
#define MMU_RW          (0x3 << MMU_AP0_BIT)

/* Memory attributes when TEX[2] == 0 */
#define MMU_STRONGLY_ORDERED    ((0 << MMU_TEX_BIT) | (0 << MMU_BC_BIT))
#define MMU_SHAREABLE_DEVICE    ((0 << MMU_TEX_BIT) | (1 << MMU_BC_BIT))
#define MMU_WT_NOWA             ((0 << MMU_TEX_BIT) | (2 << MMU_BC_BIT))
#define MMU_WB_NOWA             ((0 << MMU_TEX_BIT) | (3 << MMU_BC_BIT))
#define MMU_NON_CACHEABLE       ((1 << MMU_TEX_BIT) | (0 << MMU_BC_BIT))
#define MMU_WB_WA               ((1 << MMU_TEX_BIT) | (3 << MMU_BC_BIT))
#define MMU_NONSHAREABLE_DEVICE ((2 << MMU_TEX_BIT) | (0 << MMU_BC_BIT))

/*
 * Memory attributes when TEX[2] == 1. In this mode
 * TEX[1:0] define the outer cache attributes and
 * C, B define the inner cache attributes
 */
#define MMU_MEM_ATTR(_O, _I)    (((4 | (_O)) << MMU_TEX_BIT) | ((_I) << MMU_BC_BIT))
#define MMU_MEM_ATTR_NC         0
#define MMU_MEM_ATTR_WB_WA      1
#define MMU_MEM_ATTR_WT_NOWA    2
#define MMU_MEM_ATTR_WB_NOWA    3

#define MMU_SHAREABLE           (1 << MMU_S_BIT)
#define MMU_NORMAL_MEMORY       (MMU_MEM_ATTR(MMU_MEM_ATTR_WB_WA, MMU_MEM_ATTR_WB_WA) | MMU_SHAREABLE)
#define MMU_DEVICE_MEMORY       (MMU_SHAREABLE_DEVICE)
#define MMU_NORMAL_FLAGS        (MMU_TYPE_1MB | MMU_RW | MMU_NORMAL_MEMORY)
#define MMU_DEVICE_FLAGS        (MMU_TYPE_1MB | MMU_RW | MMU_DEVICE_MEMORY)

#endif
#endif

#endif
