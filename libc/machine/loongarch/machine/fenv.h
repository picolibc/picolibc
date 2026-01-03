/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
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

#ifndef _MACHINE_FENV_H
#define _MACHINE_FENV_H

#include <stddef.h>

typedef unsigned int fenv_t;
typedef unsigned int fexcept_t;

#ifndef __loongarch_soft_float

/* LoongArch FPU floating point control register bits.
 *
 * 31-29  -> reserved (read as 0, can not changed by software)
 * 28     -> cause bit for invalid exception
 * 27     -> cause bit for division by zero exception
 * 26     -> cause bit for overflow exception
 * 25     -> cause bit for underflow exception
 * 24     -> cause bit for inexact exception
 * 23-21  -> reserved (read as 0, can not changed by software)
 * 20     -> flag invalid exception
 * 19     -> flag division by zero exception
 * 18     -> flag overflow exception
 * 17     -> flag underflow exception
 * 16     -> flag inexact exception
 *  9-8   -> rounding control
 *  7-5   -> reserved (read as 0, can not changed by software)
 *  4     -> enable exception for invalid exception
 *  3     -> enable exception for division by zero exception
 *  2     -> enable exception for overflow exception
 *  1     -> enable exception for underflow exception
 *  0     -> enable exception for inexact exception
 *
 *
 * Rounding Control:
 * 00 - rounding ties to even (RNE)
 * 01 - rounding toward zero (RZ)
 * 10 - rounding (up) toward plus infinity (RP)
 * 11 - rounding (down) toward minus infinity (RM)
 */

/* Masks for interrupts.  */
#define _FCSR_CAUSE_LSHIFT  8
#define _FCSR_ENABLE_RSHIFT 16
#define FE_INEXACT          0x010000
#define FE_UNDERFLOW        0x020000
#define FE_OVERFLOW         0x040000
#define FE_DIVBYZERO        0x080000
#define FE_INVALID          0x100000

/* Flush denormalized numbers to zero.  */
#define FE_ALL_EXCEPT (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)

/* Rounding control.  */
#define FE_TONEAREST    0x000
#define FE_TOWARDZERO   0x100
#define FE_UPWARD       0x200
#define FE_DOWNWARD     0x300

#define FE_RMODE_MASK   0x300

#define _movfcsr2gr(cw) __asm__ volatile("movfcsr2gr %0,$fcsr0" : "=r"(cw))
#define _movgr2fcsr(cw) __asm__ volatile("movgr2fcsr $fcsr0,%0" : : "r"(cw))

#else
#define FE_TONEAREST 0
#endif

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define __declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#ifndef __loongarch_soft_float
#include <machine/fenv-fp.h>
#else
#include <machine/fenv-softfloat.h>
#endif
#endif

#endif /* _MACHINE_FENV_H */
