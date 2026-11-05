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
 * Values stay in whichever registers hold them. ACLE's __ARM_FP
 * reports what the FPU supports: bit 2 for single precision, bit 3 for
 * double precision. A float lives in a single precision VFP register
 * ("t") when the FPU has single precision, and a double in a double
 * precision register ("w") when it has double precision. Without FPU
 * support for a type, software floating point keeps it in core
 * registers ("r", a register pair for double). long double is the same
 * as double.
 */

#if defined(__ARM_FP) && (__ARM_FP & 0x4)
#define __OPT_BARRIER_FLOAT_F "+t"
#else
#define __OPT_BARRIER_FLOAT_F "+r"
#endif

#if defined(__ARM_FP) && (__ARM_FP & 0x8)
#define __OPT_BARRIER_FLOAT_D "+w"
#else
#define __OPT_BARRIER_FLOAT_D "+r"
#endif

#define __OPT_BARRIER_FLOAT_L __OPT_BARRIER_FLOAT_D

#endif /* _MACHINE_OPT_BARRIER_H_ */
