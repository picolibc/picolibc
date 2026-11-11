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
 * Values stay in whichever registers hold them. __loongarch_frlen is
 * the width of the floating point registers: 0 without an FPU, 32 for
 * single precision, 64 for double precision. A type that fits lives in
 * a floating point register ("f"); otherwise it is handled in software
 * in general registers ("r"). long double is 128 bits and always
 * handled in software, so it uses the generic memory barrier.
 */

#if defined(__loongarch_frlen) && __loongarch_frlen >= 32
#define __OPT_BARRIER_FLOAT_F "+f"
#else
#define __OPT_BARRIER_FLOAT_F "+r"
#endif

#if defined(__loongarch_frlen) && __loongarch_frlen >= 64
#define __OPT_BARRIER_FLOAT_D "+f"
#else
#define __OPT_BARRIER_FLOAT_D "+r"
#endif

#endif /* _MACHINE_OPT_BARRIER_H_ */
