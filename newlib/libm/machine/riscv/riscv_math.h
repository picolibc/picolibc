/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Kito Cheng
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

#ifndef __RISCV_MATH_H
#define __RISCV_MATH_H



#if defined(__riscv_flen) || defined(__riscv_zfinx)

#if (__riscv_flen >= 64) || defined(__riscv_zdinx)
#define __RISCV_HARD_FLOAT 64
#else
#define __RISCV_HARD_FLOAT 32
#endif

#define FCLASS_NEG_INF       (1 << 0)
#define FCLASS_NEG_NORMAL    (1 << 1)
#define FCLASS_NEG_SUBNORMAL (1 << 2)
#define FCLASS_NEG_ZERO      (1 << 3)
#define FCLASS_POS_ZERO      (1 << 4)
#define FCLASS_POS_SUBNORMAL (1 << 5)
#define FCLASS_POS_NORMAL    (1 << 6)
#define FCLASS_POS_INF       (1 << 7)
#define FCLASS_SNAN          (1 << 8)
#define FCLASS_QNAN          (1 << 9)


#define FCLASS_INF           (FCLASS_NEG_INF | FCLASS_POS_INF)
#define FCLASS_ZERO          (FCLASS_NEG_ZERO | FCLASS_POS_ZERO)
#define FCLASS_NORMAL        (FCLASS_NEG_NORMAL | FCLASS_POS_NORMAL)
#define FCLASS_SUBNORMAL     (FCLASS_NEG_SUBNORMAL | FCLASS_POS_SUBNORMAL)
#define FCLASS_NAN           (FCLASS_SNAN | FCLASS_QNAN)

#if (__riscv_flen >= 64) || defined(__riscv_zdinx)
static inline long _fclass_d(double x){
  long fclass;
  __asm __volatile ("fclass.d\t%0, %1" : "=r" (fclass) : "f" (x));
  return fclass;
}
#endif

#if (__riscv_flen >= 32) || defined(__riscv_zfinx)
static inline long _fclass_f(float x){
  long fclass;
  __asm __volatile ("fclass.s\t%0, %1" : "=r" (fclass) : "f" (x));
  return fclass;
}
#endif

#endif /* __riscv_flen */


#endif /* __RISCV_MATH_H */
