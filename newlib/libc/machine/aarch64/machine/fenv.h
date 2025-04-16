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

_BEGIN_STD_C

typedef	__uint64_t	fenv_t;
typedef	__uint64_t	fexcept_t;

/*
 * We can't tell if we're using compiler-rt or libgcc; instead
 * we assume a connection between the compiler in use and
 * the runtime library.
 */
#if defined(__clang__)

/* compiler-rt has no exceptions in soft-float */

#define __LONG_DOUBLE_NOROUND
#define __LONG_DOUBLE_NOEXCEPT

#if (__ARM_FP & 0x8) == 0
#define __DOUBLE_NOROUND
#define __DOUBLE_NOEXCEPT
#endif

#if (__ARM_FP & 0x4) == 0
#define __FLOAT_NOROUND
#define __FLOAT_NOEXCEPT
#endif

#else

/* libgcc has exceptions when there is an FPU */

#if __ARM_FP == 0
#define __LONG_DOUBLE_NOROUND
#define __LONG_DOUBLE_NOEXCEPT
#define __DOUBLE_NOROUND
#define __DOUBLE_NOEXCEPT
#define __FLOAT_NOROUND
#define __FLOAT_NOEXCEPT
#endif

#endif

#if __ARM_FP

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

/* We need to be able to map status flag positions to mask flag positions */
#define _FPUSW_SHIFT	8
#define	_ENABLE_MASK	(FE_ALL_EXCEPT << _FPUSW_SHIFT)

#define	__mrs_fpcr(__r)	__asm__ __volatile__("mrs %0, fpcr" : "=r" (__r))
#define	__msr_fpcr(__r)	__asm__ __volatile__("msr fpcr, %0" : : "r" (__r))

#define	__mrs_fpsr(__r)	__asm__ __volatile__("mrs %0, fpsr" : "=r" (__r))
#define	__msr_fpsr(__r)	__asm__ __volatile__("msr fpsr, %0" : : "r" (__r))

#else
#define	FE_TONEAREST		0x00000000
#endif

#if !defined(__declare_fenv_inline) && defined(__declare_extern_inline)
#define	__declare_fenv_inline(type) __declare_extern_inline(type)
#endif

#ifdef __declare_fenv_inline
#if __ARM_FP
#include <machine/fenv-fp.h>
#else
#include <machine/fenv-softfloat.h>
#endif

#endif

_END_STD_C

#endif	/* !_FENV_H_ */
