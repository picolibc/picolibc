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
 * Machine-specific optimization barriers; see <opt-barrier.h>. A
 * machine version of this header may choose the asm constraint the
 * barrier for each type uses:
 *
 *	__OPT_BARRIER_FLOAT_F, __OPT_BARRIER_FLOAT_D, __OPT_BARRIER_FLOAT_L
 *	__OPT_BARRIER_INT_I, __OPT_BARRIER_INT_UI, __OPT_BARRIER_INT_L,
 *	__OPT_BARRIER_INT_UL, __OPT_BARRIER_INT_LL, __OPT_BARRIER_INT_ULL
 *
 * or, if a constraint isn't enough, supply the whole function for a
 * type, with a macro of the same name:
 *
 *	__barrier_float_f, __barrier_float_d, __barrier_float_l
 *	__barrier_int_i, __barrier_int_ui, __barrier_int_l,
 *	__barrier_int_ul, __barrier_int_ll, __barrier_int_ull
 *
 * <opt-barrier.h> supplies generic versions of everything else, and
 * builds the optional opt_ barriers on top of them. This generic
 * version defines none of them.
 */

#endif /* _MACHINE_OPT_BARRIER_H_ */
