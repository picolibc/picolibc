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

#ifndef _SYS_ASM_H
#define _SYS_ASM_H

/*
 * Macros to handle different pointer/register sizes for 32/64-bit code
 */
#if __loongarch_grlen == 64
# define PTRLOG 3
# define SZREG	8
# define REG_S st.d
# define REG_L ld.d
# define ADDI addi.d
# define ADD  add.d
#elif __loongarch_grlen == 32
# define PTRLOG 2
# define SZREG	4
# define REG_S st.w
# define REG_L ld.w
# define ADDI addi.w
# define ADD  add.w
#else
# error __loongarch_grlen must equal 32 or 64
#endif

#ifndef __loongarch_soft_float
# if defined(__loongarch_single_float)
#  define SZFREG 4
#  define FREG_L fld.s
#  define FREG_S fst.s
# elif defined(__loongarch_double_float)
#  define SZFREG 8
#  define FREG_L fld.d
#  define FREG_S fst.d
# else
#  error unsupported FRLEN
# endif
#endif

#endif /* sys/asm.h */
