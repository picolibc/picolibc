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

__declare_fenv_inline(int)
feclearexcept(int excepts)
{
        (void) excepts;
	return (0);
}

__declare_fenv_inline(int)
fegetexceptflag(fexcept_t *flagp, int excepts)
{
        (void) excepts;
        (void) flagp;
	return (0);

}

__declare_fenv_inline(int)
fesetexceptflag(const fexcept_t *flagp, int excepts)
{
        (void) excepts;
        (void) flagp;
	return (0);
}

__declare_fenv_inline(int)
feraiseexcept(int excepts)
{
	return( excepts != 0 );
}

__declare_fenv_inline(int)
fesetexcept(int excepts)
{
        return( excepts != 0 );
}

__declare_fenv_inline(int)
fetestexcept(int excepts)
{
        (void) excepts;
	return (0);
}

__declare_fenv_inline(int)
fegetround(void)
{

#ifdef FE_TONEAREST
	return FE_TONEAREST;
#else
	return 0;
#endif

}

__declare_fenv_inline(int)
fesetround(int rounding_mode)
{
        (void) rounding_mode;

	return (0);
}

__declare_fenv_inline(int)
fegetenv(fenv_t *envp)
{
        (void) envp;
	return (0);
}

__declare_fenv_inline(int)
feholdexcept(fenv_t *envp)
{
        (void) envp;
	return (0);
}

__declare_fenv_inline(int)
fesetenv(const fenv_t *envp)
{
        (void) envp;
	return (0);
}

__declare_fenv_inline(int)
feupdateenv(const fenv_t *envp)
{
        (void) envp;

#if defined FE_NOMASK_ENV && FE_ALL_EXCEPT != 0
	  if (envp == FE_NOMASK_ENV)
	      return 1;
#endif

	return (0);
}

#if __BSD_VISIBLE

__declare_fenv_inline(int)
feenableexcept(int __mask)
{
        (void) __mask;
	return (0);
}

__declare_fenv_inline(int)
fedisableexcept(int __mask)
{
        (void) __mask;
	return (0);
}

__declare_fenv_inline(int)
fegetexcept(void)
{

	return (0);
}

#endif /* __BSD_VISIBLE */
