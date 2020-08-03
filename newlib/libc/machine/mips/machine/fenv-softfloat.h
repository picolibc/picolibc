/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2004-2011 David Schultz <das@FreeBSD.ORG>
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

/*
 * This file implements the functionality of <fenv.h> on platforms that
 * lack an FPU and use softfloat in libc for floating point.  To use it,
 * you must write an <fenv.h> that provides the following:
 *
 *   - a typedef for fenv_t, which may be an integer or struct type
 *   - a typedef for fexcept_t (XXX This file assumes fexcept_t is a
 *     simple integer type containing the exception mask.)
 *   - definitions of FE_* constants for the five exceptions and four
 *     rounding modes in IEEE 754, as described in fenv(3)
 *   - a definition, and the corresponding external symbol, for FE_DFL_ENV
 *   - a macro __set_env(env, flags, mask, rnd), which sets the given fenv_t
 *     from the exception flags, mask, and rounding mode
 *   - macros __env_flags(env), __env_mask(env), and __env_round(env), which
 *     extract fields from an fenv_t
 *   - a definition of __fenv_static
 *
 * If the architecture supports an optional FPU, it's recommended that you
 * define fenv_t and fexcept_t to match the hardware ABI.  Otherwise, it
 * doesn't matter how you define them.
 */
#include <errno.h>
int __softfloat_float_exception_flags;
int __softfloat_float_exception_mask;
int __softfloat_float_rounding_mode;


__fenv_static inline int
feclearexcept(int excepts)
{

	__softfloat_float_exception_flags &= ~excepts;
	return (0);
}

__fenv_static inline int
fegetexceptflag(fexcept_t *flagp, int excepts)
{

	*flagp = __softfloat_float_exception_flags & excepts;
	return (0);
}

__fenv_static inline int
fesetexceptflag(const fexcept_t *flagp, int excepts)
{

	__softfloat_float_exception_flags &= ~excepts;
	__softfloat_float_exception_flags |= *flagp & excepts;
	return (0);
}

__fenv_static inline int
feraiseexcept(int excepts)
{

return(excepts  ?  -ENOTSUP : 0 );

}

__fenv_static inline int
fetestexcept(int excepts)
{

	return (__softfloat_float_exception_flags & excepts);
}

__fenv_static inline int
fegetround(void)
{

	return (__softfloat_float_rounding_mode);
}

__fenv_static inline int
fesetround(int rounding_mode)
{

	__softfloat_float_rounding_mode = rounding_mode;
	return (0);
}

__fenv_static inline int
fegetenv(fenv_t *envp)
{

	__set_env(*envp, __softfloat_float_exception_flags,
	    __softfloat_float_exception_mask, __softfloat_float_rounding_mode);
	return (0);
}

__fenv_static inline int
feholdexcept(fenv_t *envp)
{
	fenv_t __env;

	fegetenv(envp);
	__softfloat_float_exception_flags = 0;
	__softfloat_float_exception_mask = 0;
	return (0);
}

__fenv_static inline int
fesetenv(const fenv_t *envp)
{

	__softfloat_float_exception_flags = __env_flags(*envp);
	__softfloat_float_exception_mask = __env_mask(*envp);
	__softfloat_float_rounding_mode = __env_round(*envp);
	return (0);
}

__fenv_static inline int
feupdateenv(const fenv_t *envp)
{
	int __oflags = __softfloat_float_exception_flags;

	fesetenv(envp);
	feraiseexcept(__oflags);
	return (0);
}

#if __BSD_VISIBLE

/* We currently provide no external definitions of the functions below. */

__fenv_static inline int
feenableexcept(int __mask)
{
	int __omask = __softfloat_float_exception_mask;

	__softfloat_float_exception_mask |= __mask;
	return (__omask);
}

__fenv_static inline int
fedisableexcept(int __mask)
{
	int __omask = __softfloat_float_exception_mask;

	__softfloat_float_exception_mask &= ~__mask;
	return (__omask);
}

__fenv_static inline int
fegetexcept(void)
{

	return (__softfloat_float_exception_mask);
}

#endif /* __BSD_VISIBLE */
