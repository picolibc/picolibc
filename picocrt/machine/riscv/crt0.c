/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Sebastian Meyer
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

#include "../../crt0.h"

static void __attribute__((used)) __section(".init")
_cstart(void)
{
	__start();
}

void __attribute__((naked)) __section(".text.init.enter") __attribute__((used))
_start(void)
{

	/**
	 * seems clang has no option "nopic". Now this could be problematic,
	 * since according to the clang devs at [0], that option has an effect
	 * on `la`. However, the resulting crt0.o looks the same as the one from
	 * gcc (same opcodes + pc relative relocations where I used `la`), so
	 * this could be okay.
	 * [0] https://reviews.llvm.org/D55325
	 */
#ifndef __clang__
        __asm__(".option	nopic");
#endif

	__asm__(".option	push\n"
                ".option	norelax\n"
                "la	sp, __stack\n"
                "la	gp, __global_pointer$\n"
                ".option	pop");

#ifdef __riscv_flen
	__asm__("csrr	t0, mstatus\n"
                "li	t1, 8192\n"     	// 1 << 13 = 8192
                "or	t0, t1, t0\n"
                "csrw	mstatus, t0\n"
                "csrwi	fcsr, 0");
#endif
        __asm__("j      _cstart");
}
