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

#ifndef	_SYS_FENV_H_
#define	_SYS_FENV_H_

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef	int	fenv_t;
typedef	int	fexcept_t;

/* Exception flags */
#define	FE_INEXACT	0x02000000
#define	FE_DIVBYZERO	0x04000000
#define	FE_UNDERFLOW	0x08000000
#define	FE_OVERFLOW	0x10000000
#define	FE_INVALID	0x20000000	/* all types of invalid FP ops */

/*
 * The PowerPC architecture has extra invalid flags that indicate the
 * specific type of invalid operation occurred.  These flags may be
 * tested, set, and cleared---but not masked---separately.  All of
 * these bits are cleared when FE_INVALID is cleared, but only
 * FE_VXSOFT is set when FE_INVALID is explicitly set in software.
 */
#define	FE_VXCVI	0x00000100	/* invalid integer convert */
#define	FE_VXSQRT	0x00000200	/* square root of a negative */
#define	FE_VXSOFT	0x00000400	/* software-requested exception */
#define	FE_VXVC		0x00080000	/* ordered comparison involving NaN */
#define	FE_VXIMZ	0x00100000	/* inf * 0 */
#define	FE_VXZDZ	0x00200000	/* 0 / 0 */
#define	FE_VXIDI	0x00400000	/* inf / inf */
#define	FE_VXISI	0x00800000	/* inf - inf */
#define	FE_VXSNAN	0x01000000	/* operation on a signalling NaN */
#define	FE_ALL_INVALID	(FE_VXCVI | FE_VXSQRT | FE_VXSOFT | FE_VXVC | \
			 FE_VXIMZ | FE_VXZDZ | FE_VXIDI | FE_VXISI | \
			 FE_VXSNAN | FE_INVALID)
#define	FE_ALL_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | \
			 FE_ALL_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

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
#define	_FPUSW_SHIFT	22
#define	_ENABLE_MASK	((FE_DIVBYZERO | FE_INEXACT | FE_INVALID | \
			 FE_OVERFLOW | FE_UNDERFLOW) >> _FPUSW_SHIFT)

#ifndef _SOFT_FLOAT
#ifdef __SPE__
#define	__mffs(__env) \
	__asm __volatile("mfspr %0, 512" : "=r" ((__env)->__bits.__reg))
#define	__mtfsf(__env) \
	__asm __volatile("mtspr 512,%0;isync" :: "r" ((__env).__bits.__reg))
#else
#define	__mffs(__env) \
	__asm __volatile("mffs %0" : "=f" ((__env)->__d))
#define	__mtfsf(__env) \
	__asm __volatile("mtfsf 255,%0" :: "f" ((__env).__d))
#endif
#else
#define	__mffs(__env)
#define	__mtfsf(__env)
#endif

union __fpscr {
	double __d;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		fenv_t __reg;
		__uint32_t __junk;
#else
		__uint32_t __junk;
		fenv_t __reg;
#endif
	} __bits;
};

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define	__declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#include <machine/fenv-fp.h>
#endif

#ifdef __cplusplus
}
#endif

#endif	/* !_SYS_FENV_H_ */
