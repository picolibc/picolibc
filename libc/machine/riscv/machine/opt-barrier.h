/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#ifndef _MACHINE_OPT_BARRIER_H_
#define _MACHINE_OPT_BARRIER_H_

/*
 * asm constraints for the barriers; see <opt-barrier.h>.
 *
 * Values stay in whichever registers hold them. __riscv_flen is the
 * width of the floating point registers: 32 with F, 64 with D. A type
 * that fits lives in a floating point register ("f"). Otherwise it is
 * handled in software, or by Zfinx and Zdinx, in integer registers
 * ("r"): one register when it fits in XLEN, a register pair for double
 * on rv32. long double is 128 bits and always handled in software, so
 * it uses the generic memory barrier. (Compilers don't put long double
 * in floating point registers even with the Q extension.)
 */

#if defined(__riscv_flen) && __riscv_flen >= 32
#define __OPT_BARRIER_FLOAT_F "+f"
#else
#define __OPT_BARRIER_FLOAT_F "+r"
#endif

#if defined(__riscv_flen) && __riscv_flen >= 64
#define __OPT_BARRIER_FLOAT_D "+f"
#else
#define __OPT_BARRIER_FLOAT_D "+r"
#endif

#endif /* _MACHINE_OPT_BARRIER_H_ */
