/*-
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

#ifndef	_FENV_H_
#define	_FENV_H_

#include <sys/_types.h>

#ifndef	__fenv_static
#define	__fenv_static	static
#endif

typedef	__uint64_t	fenv_t;
typedef	__uint64_t	fexcept_t;

/* Exception flags */
#define	FE_INVALID	0x00000001
#define	FE_DIVBYZERO	0x00000002
#define	FE_OVERFLOW	0x00000004
#define	FE_UNDERFLOW	0x00000008
#define	FE_INEXACT	0x00000010
#define	FE_ALL_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | \
			 FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

/*
 * Rounding modes
 *
 * We can't just use the hardware bit values here, because that would
 * make FE_UPWARD and FE_DOWNWARD negative, which is not allowed.
 */
#define	FE_TONEAREST	0x0
#define	FE_UPWARD	0x1
#define	FE_DOWNWARD	0x2
#define	FE_TOWARDZERO	0x3
#define	_ROUND_MASK	(FE_TONEAREST | FE_DOWNWARD | \
			 FE_UPWARD | FE_TOWARDZERO)
#define	_ROUND_SHIFT	22



/* Default floating-point environment */
extern const fenv_t	*_fe_dfl_env;
#define	FE_DFL_ENV	_fe_dfl_env

/* We need to be able to map status flag positions to mask flag positions */
#define _FPUSW_SHIFT	8
#define	_ENABLE_MASK	(FE_ALL_EXCEPT << _FPUSW_SHIFT)

#define	__mrs_fpcr(__r)	__asm __volatile("mrs %0, fpcr" : "=r" (__r))
#define	__msr_fpcr(__r)	__asm __volatile("msr fpcr, %0" : : "r" (__r))

#define	__mrs_fpsr(__r)	__asm __volatile("mrs %0, fpsr" : "=r" (__r))
#define	__msr_fpsr(__r)	__asm __volatile("msr fpsr, %0" : : "r" (__r))


#if __BSD_VISIBLE

/* We currently provide no external definitions of the functions below. */

static inline int
feenableexcept(int __mask)
{
	fenv_t __old_r, __new_r;

	__mrs_fpcr(__old_r);
	__new_r = __old_r | ((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	__msr_fpcr(__new_r);
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

static inline int
fedisableexcept(int __mask)
{
	fenv_t __old_r, __new_r;

	__mrs_fpcr(__old_r);
	__new_r = __old_r & ~((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	__msr_fpcr(__new_r);
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

static inline int
fegetexcept(void)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	return ((__r & _ENABLE_MASK) >> _FPUSW_SHIFT);
}

#endif /* __BSD_VISIBLE */



#endif	/* !_FENV_H_ */
