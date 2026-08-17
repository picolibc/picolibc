/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2000-2026 MIPS Holding, Inc
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

#define _ISOC23_SOURCE
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#if defined(__has_include) && __has_include(<stdckdint.h>)
#include <stdckdint.h>

int
main(void)
{
    uint8_t  u8;
    uint16_t u16;
    uint32_t u32;
    int      i;
    int      overflow;

    overflow = ckd_add(&u16, (uint16_t)14, (uint16_t)28);
    if (overflow || u16 != 42) {
        printf("ckd_add u16 ok case failed: overflow=%d u16=%u\n", overflow, (unsigned)u16);
        return 1;
    }

    overflow = ckd_add(&u8, (uint8_t)200, (uint8_t)100);
    if (!overflow || u8 != 44) {
        printf("ckd_add u8 overflow case failed: overflow=%d u8=%u\n", overflow, (unsigned)u8);
        return 1;
    }

    overflow = ckd_add(&u32, (uint8_t)200, (uint8_t)100);
    if (overflow || u32 != 300) {
        printf("ckd_add u32 wide case failed: overflow=%d u32=%u\n", overflow, (unsigned)u32);
        return 1;
    }

    overflow = ckd_add(&i, INT_MAX, 1);
    if (!overflow) {
        printf("ckd_add INT_MAX+1 failed: overflow=%d i=%d\n", overflow, i);
        return 1;
    }

    overflow = ckd_sub(&u16, (uint16_t)50, (uint16_t)8);
    if (overflow || u16 != 42) {
        printf("ckd_sub u16 ok case failed: overflow=%d u16=%u\n", overflow, (unsigned)u16);
        return 1;
    }

    overflow = ckd_sub(&u8, (uint8_t)5, (uint8_t)10);
    if (!overflow || u8 != (uint8_t)(5u - 10u)) {
        printf("ckd_sub u8 overflow case failed: overflow=%d u8=%u\n", overflow, (unsigned)u8);
        return 1;
    }

    overflow = ckd_mul(&u16, (uint16_t)6, (uint16_t)7);
    if (overflow || u16 != 42) {
        printf("ckd_mul u16 ok case failed: overflow=%d u16=%u\n", overflow, (unsigned)u16);
        return 1;
    }

    overflow = ckd_mul(&u8, (uint8_t)20, (uint8_t)20);
    if (!overflow || u8 != (uint8_t)(20u * 20u)) {
        printf("ckd_mul u8 overflow case failed: overflow=%d u8=%u\n", overflow, (unsigned)u8);
        return 1;
    }

    printf("stdckdint tests passed\n");
    return 0;
}
#else
int
main(void)
{
    return 77;
}
#endif