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

#ifndef _ARC_SEMIHOST_H_
#define _ARC_SEMIHOST_H_

#include <stdint.h>
#include <errno.h>

#define SYS_SEMIHOST_exit	1
#define SYS_SEMIHOST_read	3
#define SYS_SEMIHOST_write	4
#define SYS_SEMIHOST_open	5
#define SYS_SEMIHOST_close	6
#define SYS_SEMIHOST_unlink	10
#define SYS_SEMIHOST_time	13
#define SYS_SEMIHOST_lseek	19
#define SYS_SEMIHOST_times	43
#define SYS_SEMIHOST_gettimeofday	78
#define SYS_SEMIHOST_stat	106 /* nsim stat's is corupted.  */
#define SYS_SEMIHOST_fstat	108
#define SYS_SEMIHOST_argc	1000
#define SYS_SEMIHOST_argv_sz	1001
#define SYS_SEMIHOST_argv	1002
#define SYS_SEMIHOST_memset	1004
#define SYS_SEMIHOST_errno      2000

#ifdef __ARC700__
#define SWI     "trap0"
#else
#define SWI     "swi"
#endif

static inline uintptr_t
arc_semihost0(uintptr_t op)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0");

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op));
    return r_r0;
}

static inline uintptr_t
arc_semihost1(uintptr_t op, uintptr_t r0)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0") = r0;

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op), "r" (r_r0));
    return r_r0;
}

static inline uintptr_t
arc_semihost2(uintptr_t op, uintptr_t r0, uintptr_t r1)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0") = r0;
    register uintptr_t r_r1 __asm__("r1") = r1;

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op), "r" (r_r0), "r" (r_r1));
    return r_r0;
}

static inline uintptr_t
arc_semihost3(uintptr_t op, uintptr_t r0, uintptr_t r1, uintptr_t r2)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0") = r0;
    register uintptr_t r_r1 __asm__("r1") = r1;
    register uintptr_t r_r2 __asm__("r2") = r2;

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op), "r" (r_r0), "r" (r_r1), "r" (r_r2));
    return r_r0;
}

static inline void
arc_semihost_errno(int def)
{
    int err = arc_semihost0(SYS_SEMIHOST_errno);
    if (err < 0)
        err = def;
    errno = err;
}

#endif /* _ARC_SEMIHOST_H_ */
