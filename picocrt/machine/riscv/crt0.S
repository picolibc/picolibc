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

#include <picolibc.h>	

#ifndef CONSTRUCTORS
#define CONSTRUCTORS 1
#endif

	/**
	 * seems clang has no option "nopic". Now this could be problematic,
	 * since according to the clang devs at [0], that option has an effect
	 * on `la`. However, the resulting crt0.o looks the same as the one from
	 * gcc (same opcodes + pc relative relocations where I used `la`), so
	 * this could be okay.
	 * [0] https://reviews.llvm.org/D55325
	 */
#ifndef __clang__
	.option	nopic
#endif

	.text
	.section	.text.init.enter,"ax",@progbits
	.align	1
	.globl	_start
	.type	_start, @function
_start:
	.option	push
	.option	norelax
	la	sp, __stack
	la	gp, __global_pointer$
	.option	pop

#ifdef __riscv_flen
	csrr	t0, mstatus
	// 1 << 13 = 8192
	li	t1, 8192
	or	t0, t1, t0
	csrw	mstatus, t0
	csrwi	fcsr, 0
#endif

	// __start

	// call to memcpy(__data_start, __data_source, __data_size)
	la	a0, __data_start
	la	a1, __data_source
	la	a2, __data_size
	call	memcpy

	// call to memset(__bss_start, '\0', __bss_size)
	la	a0, __bss_start
	li	a1, 0
	la	a2, __bss_size
	call	memset

#ifdef PICOLIBC_TLS
	// call to _set_tls(__tls_base)
	la	a0, __tls_base
	call	_set_tls
#endif

#if defined(HAVE_INITFINI_ARRAY) && CONSTRUCTORS
	call	__libc_init_array
#endif

	// call to main(0, NULL)
	li	a0, 0
	li	a1, 0
	call	main

#ifdef CRT0_EXIT
	call exit
#else
.L1:	
	j	.L1
#endif
