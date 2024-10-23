/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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
#include <machine/core-isa.h>

	.begin		no-absolute-literals
	.section	.reset.entry,"ax"
	.align		4
	.globl		_start
	.type		_start, @function
_start:
	j		_reset_handler
	.size		_start, . - _start

	.section	.reset.text,"ax"
	.literal_position
	.align		4
	.global 	_reset_handler
_reset_handler:

	movi		sp, __stack
	
	movi		a0, 0
	wsr		a0, INTENABLE
#if XCHAL_HAVE_ABSOLUTE_LITERALS
	wsr		a0, LITBASE
#endif
	rsync

#if XCHAL_HAVE_S32C1I && (XCHAL_HW_MIN_VERSION >= XTENSA_HWVERSION_RC_2009_0)
#if XCHAL_DCACHE_IS_COHERENT
	/* MX -- internal for writeback, RCW otherwise */
	movi	a3, 0x25
#else
	/* non-MX -- always RCW */
	movi	a3, 0x15
#endif /* XCHAL_DCACHE_IS_COHERENT */
	wsr	a3, ATOMCTL
#endif

#define PS_INTLEVEL_MASK      	0x0000000F
#define PS_INTLEVEL(n)		((n)&PS_INTLEVEL_MASK)
#define PS_WOE			0x00040000
#define PS_UM			0x00000020

# ifdef __XTENSA_CALL0_ABI__
	/*
	 * PS.WOE = 0
	 * PS.UM = 0
	 * PS.EXCM = 0
	 * PS.INTLEVEL = 0
	 */
	movi	a3, PS_INTLEVEL(0)
# else
	movi		a2, 1
	wsr		a2, WINDOWSTART
	wsr		a0, WINDOWBASE
	rsync
	movi		a0, 0

	/*
	 * PS.WOE = 1
	 * PS.UM = 0
	 * PS.EXCM = 0
	 * PS.INTLEVEL = 0
	 */
	movi	a3, PS_WOE|PS_INTLEVEL(0)
# endif
	wsr	a3, PS
	rsync

	movi		a0, _cstart
#ifdef __XTENSA_CALL0_ABI__
	callx0		a0
#else
	callx4		a0
#endif
	.size		_reset_handler, . - _reset_handler
	.end	no-absolute-literals

#ifndef __XTENSA_CALL0_ABI__
	.section	.vectors,"ax"

	.org		0x0
	.globl		_WindowOverflow4
	.type		_WindowOverflow4, @function
_WindowOverflow4:
	s32e		a0, a5, -16
	s32e		a1, a5, -12
	s32e		a2, a5,  -8
	s32e		a3, a5,  -4
	rfwo
	.size		_WindowOverflow4, . - _WindowOverflow4

	.org		0x40
	.globl		_WindowUnderflow4
	.type		_WindowUnderflow4, @function
_WindowUnderflow4:
	l32e		a0, a5, -16
	l32e		a1, a5, -12
	l32e		a2, a5,  -8
	l32e		a3, a5,  -4
	rfwu
	.size		_WindowUnderflow4, . - _WindowUnderflow4

	.org		0x80
	.globl		_WindowOverflow8
	.type		_WindowOverflow8, @function
_WindowOverflow8:
	s32e		a0, a9, -16
	l32e		a0, a1, -12
	s32e		a1, a9, -12
	s32e		a2, a9,  -8
	s32e		a3, a9,  -4
	s32e		a4, a0, -32
	s32e		a5, a0, -28
	s32e		a6, a0, -24
	s32e		a7, a0, -20
	rfwo
	.size		_WindowOverflow8, . - _WindowOverflow8

	.org		0xc0
	.globl		_WindowUnderflow8
	.type		_WindowUnderflow8, @function
_WindowUnderflow8:
	l32e		a0, a9, -16
	l32e		a1, a9, -12
	l32e		a2, a9,  -8
	l32e		a7, a1, -12
	l32e		a3, a9,  -4
	l32e		a4, a7, -32
	l32e		a5, a7, -28
	l32e		a6, a7, -24
	l32e		a7, a7, -20
	rfwu
	.size		_WindowUnderflow8, . - _WindowUnderflow8

	.org		0x100
	.globl		_WindowOverflow12
	.type		_WindowOverflow12, @function
_WindowOverflow12:
	s32e		a0,  a13, -16
	l32e		a0,  a1,  -12
	s32e		a1,  a13, -12
	s32e		a2,  a13,  -8
	s32e		a3,  a13,  -4
	s32e		a4,  a0,  -48
	s32e		a5,  a0,  -44
	s32e		a6,  a0,  -40
	s32e		a7,  a0,  -36
	s32e		a8,  a0,  -32
	s32e		a9,  a0,  -28
	s32e		a10, a0,  -24
	s32e		a11, a0,  -20
	rfwo
	.size		_WindowOverflow12, . - _WindowOverflow12


	.org		0x140
	.globl		_WindowUnderflow12
	.type		_WindowUnderflow12, @function
_WindowUnderflow12:
	l32e		a0,  a13, -16
	l32e		a1,  a13, -12
	l32e		a2,  a13,  -8
	l32e		a11, a1,  -12
	l32e		a3,  a13,  -4
	l32e		a4,  a11, -48
	l32e		a5,  a11, -44
	l32e		a6,  a11, -40
	l32e		a7,  a11, -36
	l32e		a8,  a11, -32
	l32e		a9,  a11, -28
	l32e		a10, a11, -24
	l32e		a11, a11, -20
	rfwu
	.size		_WindowUnderflow12, . - _WindowUnderflow12
#endif
