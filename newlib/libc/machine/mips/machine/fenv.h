/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2004-2005 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef	_MACHINE_FENV_H_
#define	_MACHINE_FENV_H_

#include <sys/cdefs.h>

#if !defined(__mips_soft_float) && !defined(__mips_hard_float)
#error compiler didnt set soft/hard float macros
#endif

_BEGIN_STD_C

typedef int fenv_t;
typedef int fexcept_t;

#ifndef __mips_soft_float

/* Exception flags */
#define	_FCSR_CAUSE_SHIFT	10
#define	FE_INVALID	0x0040
#define	FE_DIVBYZERO	0x0020
#define	FE_OVERFLOW	0x0010
#define	FE_UNDERFLOW	0x0008
#define	FE_INEXACT	0x0004
#define	FE_ALL_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | \
			 FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

/* Rounding modes */
#define	FE_TONEAREST	0x0000
#define	FE_TOWARDZERO	0x0001
#define	FE_UPWARD	0x0002
#define	FE_DOWNWARD	0x0003
#define	_ROUND_MASK	(FE_TONEAREST | FE_DOWNWARD | \
			 FE_UPWARD | FE_TOWARDZERO)

/* Default floating-point environment */
extern fenv_t		_fe_dfl_env;
#define	FE_DFL_ENV	((const fenv_t *) &_fe_dfl_env)

/* We need to be able to map status flag positions to mask flag positions */
#define	_FCSR_ENABLE_SHIFT	5
#define	_FCSR_ENABLE_MASK	(FE_ALL_EXCEPT << _FCSR_ENABLE_SHIFT)

#define	__cfc1(__fcsr)	__asm__ __volatile__("cfc1 %0, $31" : "=r" (__fcsr))
#define	__ctc1(__fcsr)	__asm__ __volatile__("ctc1 %0, $31" :: "r" (__fcsr))

#else
#define	FE_TONEAREST	0x0000
#endif /* !__mips_soft_float */

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define	__declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#ifdef	__mips_soft_float
#include <machine/fenv-softfloat.h>
#else
#include <machine/fenv-fp.h>
#endif
#endif

_END_STD_C

#endif	/* !_FENV_H_ */
