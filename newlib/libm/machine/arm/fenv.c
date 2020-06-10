/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
 * Copyright (c) 2013 Andrew Turner <andrew@FreeBSD.ORG>
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

#define	__fenv_static
#include <fenv.h>

#include <machine/acle-compat.h>

#if __ARM_ARCH >= 6
#define FENV_ARMv6
#endif

/* When SOFTFP_ABI is defined we are using the softfp ABI. */
#if defined(__VFP_FP__) && !defined(__ARM_PCS_VFP)
#define SOFTFP_ABI
#endif


#ifndef FENV_MANGLE
/*
 * Hopefully the system ID byte is immutable, so it's valid to use
 * this as a default environment.
 */
fenv_t __fe_dfl_env = { 0 };

const fenv_t *_fe_dfl_env = &__fe_dfl_env;
#endif


/* If this is a non-mangled softfp version special processing is required */
#if defined(FENV_MANGLE) || !defined(SOFTFP_ABI) || !defined(FENV_ARMv6)

/*
 * The following macros map between the softfloat emulator's flags and
 * the hardware's FPSR.  The hardware this file was written for doesn't
 * have rounding control bits, so we stick those in the system ID byte.
 */
#ifndef __ARM_PCS_VFP
#define	__set_env(env, flags, mask, rnd) env = ((flags)			\
						| (mask)<<_FPUSW_SHIFT	\
						| (rnd) << 24)
#define	__env_flags(env)		((env) & FE_ALL_EXCEPT)
#define	__env_mask(env)			(((env) >> _FPUSW_SHIFT)	\
						& FE_ALL_EXCEPT)
#define	__env_round(env)		(((env) >> 24) & _ROUND_MASK)

#include <machine/fenv-softfloat.h>

#else /* __ARM_PCS_VFP PRESENT */

#include <machine/fenv-vfp.h>

#endif /* __ARM_PCS_VFP */

#ifdef __GNUC_GNU_INLINE__
#error "This file must be compiled with C99 'inline' semantics"
#endif

extern inline int feclearexcept(int excepts);
extern inline int fegetexceptflag(fexcept_t *flagp, int excepts);
extern inline int fesetexceptflag(const fexcept_t *flagp, int excepts);
extern inline int feraiseexcept(int excepts);
extern inline int fetestexcept(int excepts);
extern inline int fegetround(void);
extern inline int fesetround(int round);
extern inline int fegetenv(fenv_t *envp);
extern inline int feholdexcept(fenv_t *envp);
extern inline int fesetenv(const fenv_t *envp);
extern inline int feupdateenv(const fenv_t *envp);
extern inline int feenableexcept(int __mask);
extern inline int fedisableexcept(int __mask);
extern inline int fegetexcept(void);

#else /* !FENV_MANGLE && SOFTFP_ABI */
/* Set by libc when the VFP unit is enabled */

int _libc_arm_fpu_present;

int __softfp_feclearexcept(int excepts);
int __softfp_fegetexceptflag(fexcept_t *flagp, int excepts);
int __softfp_fesetexceptflag(const fexcept_t *flagp, int excepts);
int __softfp_feraiseexcept(int excepts);
int __softfp_fetestexcept(int excepts);
int __softfp_fegetround(void);
int __softfp_fesetround(int round);
int __softfp_fegetenv(fenv_t *envp);
int __softfp_feholdexcept(fenv_t *envp);
int __softfp_fesetenv(const fenv_t *envp);
int __softfp_feupdateenv(const fenv_t *envp);
int __softfp_feenableexcept(int __mask);
int __softfp_fedisableexcept(int __mask);
int __softfp_fegetexcept(void);

int __vfp_feclearexcept(int excepts);
int __vfp_fegetexceptflag(fexcept_t *flagp, int excepts);
int __vfp_fesetexceptflag(const fexcept_t *flagp, int excepts);
int __vfp_feraiseexcept(int excepts);
int __vfp_fetestexcept(int excepts);
int __vfp_fegetround(void);
int __vfp_fesetround(int round);
int __vfp_fegetenv(fenv_t *envp);
int __vfp_feholdexcept(fenv_t *envp);
int __vfp_fesetenv(const fenv_t *envp);
int __vfp_feupdateenv(const fenv_t *envp);
int __vfp_feenableexcept(int __mask);
int __vfp_fedisableexcept(int __mask);
int __vfp_fegetexcept(void);

static int
__softfp_round_to_vfp(int round)
{

	switch (round) {
	case FE_TONEAREST:
	default:
		return VFP_FE_TONEAREST;
	case FE_TOWARDZERO:
		return VFP_FE_TOWARDZERO;
	case FE_UPWARD:
		return VFP_FE_UPWARD;
	case FE_DOWNWARD:
		return VFP_FE_DOWNWARD;
	}
}

static int
__softfp_round_from_vfp(int round)
{

	switch (round) {
	case VFP_FE_TONEAREST:
	default:
		return FE_TONEAREST;
	case VFP_FE_TOWARDZERO:
		return FE_TOWARDZERO;
	case VFP_FE_UPWARD:
		return FE_UPWARD;
	case VFP_FE_DOWNWARD:
		return FE_DOWNWARD;
	}
}

int feclearexcept(int excepts)
{

	if (_libc_arm_fpu_present)
		__vfp_feclearexcept(excepts);
	__softfp_feclearexcept(excepts);

	return (0);
}

int fegetexceptflag(fexcept_t *flagp, int excepts)
{
	fexcept_t __vfp_flagp;

	__vfp_flagp = 0;
	if (_libc_arm_fpu_present)
		__vfp_fegetexceptflag(&__vfp_flagp, excepts);
	__softfp_fegetexceptflag(flagp, excepts);

	*flagp |= __vfp_flagp;

	return (0);
}

int fesetexceptflag(const fexcept_t *flagp, int excepts)
{

	if (_libc_arm_fpu_present)
		__vfp_fesetexceptflag(flagp, excepts);
	__softfp_fesetexceptflag(flagp, excepts);

	return (0);
}

int feraiseexcept(int excepts)
{

	if (_libc_arm_fpu_present)
		__vfp_feraiseexcept(excepts);
	__softfp_feraiseexcept(excepts);

	return (0);
}

int fetestexcept(int excepts)
{
	int __got_excepts;

	__got_excepts = 0;
	if (_libc_arm_fpu_present)
		__got_excepts = __vfp_fetestexcept(excepts);
	__got_excepts |= __softfp_fetestexcept(excepts);

	return (__got_excepts);
}

int fegetround(void)
{

	if (_libc_arm_fpu_present)
		return __softfp_round_from_vfp(__vfp_fegetround());
	return __softfp_fegetround();
}

int fesetround(int round)
{

	if (_libc_arm_fpu_present)
		__vfp_fesetround(__softfp_round_to_vfp(round));
	__softfp_fesetround(round);

	return (0);
}

int fegetenv(fenv_t *envp)
{
	fenv_t __vfp_envp;

	__vfp_envp = 0;
	if (_libc_arm_fpu_present)
		__vfp_fegetenv(&__vfp_envp);
	__softfp_fegetenv(envp);
	*envp |= __vfp_envp;

	return (0);
}

int feholdexcept(fenv_t *envp)
{
	fenv_t __vfp_envp;

	__vfp_envp = 0;
	if (_libc_arm_fpu_present)
		__vfp_feholdexcept(&__vfp_envp);
	__softfp_feholdexcept(envp);
	*envp |= __vfp_envp;

	return (0);
}

int fesetenv(const fenv_t *envp)
{

	if (_libc_arm_fpu_present)
		__vfp_fesetenv(envp);
	__softfp_fesetenv(envp);

	return (0);
}

int feupdateenv(const fenv_t *envp)
{

	if (_libc_arm_fpu_present)
		__vfp_feupdateenv(envp);
	__softfp_feupdateenv(envp);

	return (0);
}

int feenableexcept(int __mask)
{
	int __unmasked;

	__unmasked = 0;
	if (_libc_arm_fpu_present)
		__unmasked = __vfp_feenableexcept(__mask);
	__unmasked |= __softfp_feenableexcept(__mask);

	return (__unmasked);
}

int fedisableexcept(int __mask)
{
	int __unmasked;

	__unmasked = 0;
	if (_libc_arm_fpu_present)
		__unmasked = __vfp_fedisableexcept(__mask);
	__unmasked |= __softfp_fedisableexcept(__mask);

	return (__unmasked);
}

int fegetexcept(void)
{
	int __unmasked;

	__unmasked = 0;
	if (_libc_arm_fpu_present)
		__unmasked = __vfp_fegetexcept();
	__unmasked |= __softfp_fegetexcept();

	return (__unmasked);
}

#endif

