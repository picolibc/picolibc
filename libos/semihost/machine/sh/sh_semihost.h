/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#ifndef _SH_SEMIHOST_H_
#define _SH_SEMIHOST_H_

#include <stdio.h>
#include <stdint.h>

#if defined(__SH4__) || defined(__SH4_SINGLE__) || defined(__SH4_SINGLE_ONLY__)
#define SH_QEMU
#endif

typedef volatile uint8_t  vuint8_t;
typedef volatile uint32_t vuint32_t;

#ifdef SH_QEMU

struct sh_serial {
    vuint32_t smr;
    vuint32_t brr;
    vuint32_t scr;
    vuint32_t tdr;
};

#define sh_serial0 (*(struct sh_serial *)0xffe00000)
#define sh_serial1 (*(struct sh_serial *)0xffe80000)

#else

#define TARGET_NEWLIB_SH_SYS_exit  1
#define TARGET_NEWLIB_SH_SYS_read  3
#define TARGET_NEWLIB_SH_SYS_write 4

struct sh_syscall_args {
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
};

uint32_t sh_syscall(uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7);

#endif

#endif /* _SH_SEMIHOST_H_ */
